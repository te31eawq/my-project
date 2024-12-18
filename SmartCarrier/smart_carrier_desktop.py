import os
import cv2
import torch
import numpy as np
from ultralytics import YOLO
import mediapipe as mp
from depth_anything_v2.dpt import DepthAnythingV2
from collections import deque
import time
import threading
import serial
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), 'depth_anything'))

os.environ["PYTORCH_CUDA_ALLOC_CONF"] = "expandable_segments:True"

class DepthAndPoseEstimator:
    def __init__(self, encoder='vits', video_source=1, moving_average_window_size=5):
        self.device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
        self.depth_model = self.initialize_depth_model(encoder)
        self.yolo_model = YOLO(os.path.join(os.path.dirname(__file__), 'yolov8s.pt'), verbose=False)
        self.pose = mp.solutions.pose.Pose()
        self.video_stream = VideoStream(source=video_source)
        self.depth_estimate = None
        self.saved_depth = None
        self.last_person_box = None
        self.tracked_person_box = None  
        self.depth_threshold = 0.1  
        self.depth_history = deque(maxlen=moving_average_window_size)
        self.tracked_person_box_history = deque(maxlen=5)
        self.last_command_time = time.time()
        self.stop_flag = False
        self.previous_command = None
        SERIAL_PORT = '/dev/rfcomm0'
        BAUD_RATE = 9600
        try:
            self.ser = serial.Serial(SERIAL_PORT, BAUD_RATE)
            print("Bluetooth initialized successfully.")
        except serial.SerialException as e:
            print(f"Warning: Could not open Bluetooth port {SERIAL_PORT}. Running without Bluetooth. Error: {e}")
            self.ser = None

    def clear_memory(self):
        if self.device.type == 'cuda':
            torch.cuda.empty_cache()
    
    def initialize_depth_model(self, encoder):
        model_configs = {
            'vits': {'encoder': 'vits', 'features': 64, 'out_channels': [48, 96, 192, 384]},
            'vitb': {'encoder': 'vitb', 'features': 128, 'out_channels': [96, 192, 384, 768]},
            'vitl': {'encoder': 'vitl', 'features': 256, 'out_channels': [256, 512, 1024, 1024]},
            'vitg': {'encoder': 'vitg', 'features': 384, 'out_channels': [1536, 1536, 1536, 1536]}
        }
        depth_model = DepthAnythingV2(**model_configs[encoder])
        depth_model.load_state_dict(torch.load(os.path.join(os.path.dirname(__file__), f'depth_anything_v2_{encoder}.pth'), map_location='cpu'))
        return depth_model.to(self.device).eval()

    def process_frame(self, raw_img):
        depth_scaled = None  
        person_box = self.detect_person(raw_img)

        if person_box:
            x1, y1, x2, y2 = person_box
            y1 = 0
            y2 = raw_img.shape[0]
            cropped_img = raw_img[y1:y2, x1:x2+1]
            cropped_img = (cropped_img * 255).astype(np.uint8) if cropped_img.dtype != np.uint8 else cropped_img
            depth = self.depth_model.infer_image(cropped_img)
            depth_normalized = (depth - depth.min()) / (depth.max() - depth.min() + 1e-5)
            depth_scaled = np.clip(depth_normalized * 50, 0, 255).astype(np.uint8)
            depth_scaled = cv2.GaussianBlur(depth_scaled, (5, 5), 0)
            self.last_person_box = person_box
            self.draw_person_box(raw_img, person_box, depth_scaled)
        elif self.last_person_box:
            self.draw_person_box(raw_img, self.last_person_box, depth_scaled)

        return raw_img, depth_scaled

    def detect_person(self, raw_img):
        results = self.yolo_model(raw_img)
        current_boxes = []

        for result in results:
            boxes = result.boxes
            for i in range(len(boxes)):
                cls = int(boxes.cls[i])
                confidence = boxes.conf[i]
                if cls == 0 and confidence > 0.5:
                    current_boxes.append(tuple(map(int, boxes.xyxy[i])))

        # 첫 번째 사람을 초기화
        if not self.tracked_person_box and current_boxes:
            self.tracked_person_box = current_boxes[0]  # Initialize first detected person

        # IOU를 계산하여 기존 트래킹 박스를 업데이트
        if self.tracked_person_box and current_boxes:
            best_box, max_iou = None, 0
            for box in current_boxes:
                iou = self.calculate_iou(self.tracked_person_box, box)
                if iou > max_iou:
                    max_iou = iou
                    best_box = box

            # IOU가 임계값을 넘으면 기존 박스를 업데이트
            if max_iou > 0.5:  # IOU threshold
                self.tracked_person_box = best_box

        return self.tracked_person_box
    
    def calculate_iou(self, boxA, boxB):
        xA, yA = max(boxA[0], boxB[0]), max(boxA[1], boxB[1])
        xB, yB = min(boxA[2], boxB[2]), min(boxA[3], boxB[3])
        interArea = max(0, xB - xA + 1) * max(0, yB - yA + 1)

        boxAArea = (boxA[2] - boxA[0] + 1) * (boxA[3] - boxA[1] + 1)
        boxBArea = (boxB[2] - boxB[0] + 1) * (boxB[3] - boxB[1] + 1)

        return interArea / float(boxAArea + boxBArea - interArea)


    def draw_person_box(self, raw_img, person_box, depth_scaled):
        if depth_scaled is None:
            return

        x1, y1, x2, y2 = person_box
        frame_rgb = cv2.cvtColor(raw_img, cv2.COLOR_BGR2RGB)
        pose_results = self.pose.process(frame_rgb)

        if pose_results.pose_landmarks:
            h, w, _ = raw_img.shape
            left_shoulder = pose_results.pose_landmarks.landmark[mp.solutions.pose.PoseLandmark.LEFT_SHOULDER]
            right_shoulder = pose_results.pose_landmarks.landmark[mp.solutions.pose.PoseLandmark.RIGHT_SHOULDER]

            left_shoulder_visible = left_shoulder.visibility > 0.5
            right_shoulder_visible = right_shoulder.visibility > 0.5

            shoulder_middle = self.calculate_shoulder_middle(left_shoulder, right_shoulder, w, x1, x2)
            if shoulder_middle is None:
                return 

            left_shoulder_x = int(left_shoulder.x * w) if left_shoulder_visible else x1
            right_shoulder_x = int(right_shoulder.x * w) if right_shoulder_visible else x2

            cv2.rectangle(raw_img, (left_shoulder_x, y1), (right_shoulder_x, y2), (0, 255, 0), 2)

            shoulder_y = (int(left_shoulder.y * h) + int(right_shoulder.y * h)) // 2 if left_shoulder_visible and right_shoulder_visible else h // 2
            cv2.circle(raw_img, (shoulder_middle, shoulder_y), 5, (0, 0, 255), 2)

            self.update_depth_estimate_from_circle(shoulder_middle, shoulder_y, depth_scaled, x1)
            self.display_depth_info(raw_img, shoulder_middle, shoulder_y)

            section_width = w // 3
            cv2.line(raw_img, (section_width, 0), (section_width, h), (255, 255, 255), 2)
            cv2.line(raw_img, (section_width * 2, 0), (section_width * 2, h), (255, 255, 255), 2)

            # 사람 인식 중일 때 명령을 계속 보냄 (방향 전환을 우선 처리하고, 그 후 직진 또는 정지 처리)
            if shoulder_middle < section_width:
                # 왼쪽으로 이동
                cv2.putText(raw_img, 'LEFT', (10,60), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0,255,0), 2)
                self.send_command('L')
                threading.Timer(0.061, self.stop_command).start()
            elif shoulder_middle > section_width * 2:
                # 오른쪽으로 이동
                cv2.putText(raw_img, 'RIGHT', (10,60), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0,255,0), 2)
                self.send_command('R')
                threading.Timer(0.061, self.stop_command).start()
            elif self.depth_estimate is not None:
                # 방향이 맞춰졌다면, 거리 조건을 체크하여 전진 또는 정지
                if self.depth_estimate >= 40.2:
                    # 거리 36 이상일 때 직진
                    cv2.putText(raw_img, 'Straight', (10,90), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0,255,0), 2)
                    self.send_command('F')
                    threading.Timer(1.2, self.stop_command).start()
                elif self.depth_estimate < 40.2:
                    # 거리 34 이하일 때 멈춤
                    cv2.putText(raw_img, 'Stop', (10,90), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0,0,255), 2)
                    self.stop_command()
            else:
                # 사람이 인식되지 않으면 정지
                self.stop_command()


    def stop_command(self):
        self.send_command('f')
        self.stop_flag = False

    def send_command(self, command):
        if self.ser and command != self.previous_command:
            self.ser.write(command.encode())
            self.previous_command = command
            print(f"Sent command: {command}")

    def calculate_shoulder_middle(self, left_shoulder, right_shoulder, width, x1, x2):
        if left_shoulder.visibility > 0.5 and right_shoulder.visibility > 0.5:
            left_shoulder_x = int(left_shoulder.x * width)
            right_shoulder_x = int(right_shoulder.x * width)
            return (left_shoulder_x + right_shoulder_x) // 2
        elif left_shoulder.visibility > 0.5:
            return int(left_shoulder.x * width + (width - x2)) // 2
        elif right_shoulder.visibility > 0.5:
            return int(right_shoulder.x * width + x1) // 2
        return None

    def update_depth_estimate_from_circle(self, shoulder_middle, shoulder_y, depth_scaled, x1):
        cropped_center_x = shoulder_middle - x1
        if 0 <= cropped_center_x < depth_scaled.shape[1] and 0 <= shoulder_y < depth_scaled.shape[0]:
            current_depth_value = np.mean(depth_scaled[shoulder_y, cropped_center_x])
            self.update_depth_estimate(current_depth_value)
            cv2.circle(depth_scaled, (cropped_center_x, shoulder_y), 5, (0, 0, 255), 2)

    def display_depth_info(self, raw_img, shoulder_middle, shoulder_y):
        if self.depth_estimate is not None:
            depth_estimate_rounded = round(self.depth_estimate,1)
            cv2.putText(raw_img, f'Depth: {depth_estimate_rounded}', (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255, 0, 0), 2)

        cv2.circle(raw_img, (shoulder_middle, shoulder_y), 5, (0, 0, 255), 2)

    def update_depth_estimate(self, current_depth_value):
        self.depth_estimate = current_depth_value if self.depth_estimate is None else (self.depth_estimate * 0.8) + (current_depth_value * 0.2)

    def save_depth(self, current_depth_value):
        self.saved_depth = current_depth_value

    def run(self):
        if not self.video_stream.cap.isOpened():
            print("Error: Could not open video stream.")
            return

        try:
            while True:
                raw_img = self.video_stream.read_frame()
                if raw_img is None:
                    continue
                
                processed_img, depth_image = self.process_frame(raw_img)

                torch.cuda.empty_cache()

                cv2.imshow('Original Image', processed_img)
                if depth_image is not None:
                    depth_image = cv2.applyColorMap(depth_image.astype(np.uint8), cv2.COLORMAP_JET)
                    cv2.imshow('Depth Image', depth_image)

                key = cv2.waitKey(1) & 0xFF
                if key == ord('q'):
                    break

                # 만약 사람을 인식하지 않으면 정지
                if self.tracked_person_box is None:
                    self.stop_command()

        finally:
            self.video_stream.release()
            if self.ser:
                self.ser.close()
            cv2.destroyAllWindows()

class VideoStream:
    def __init__(self, source):
        self.source = source
        self.cap = self._open_video_stream()
        self.frame = None
        self.running = True
        self.thread = threading.Thread(target=self.update, daemon=True)
        self.thread.start()

    @staticmethod
    def _open_video_stream():
        gstreamer_pipeline = (
             'udpsrc port=5000 caps="application/x-rtp, payload=96" ! '
             'rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! '
             'appsink caps="video/x-raw, format=(string)BGR"'
        )
        cap = cv2.VideoCapture(gstreamer_pipeline, cv2.CAP_GSTREAMER)
        if not cap.isOpened():
            print("Error: Unable to open video stream")
        return cap

    def update(self):
        while self.running:
            ret, frame = self.cap.read()
            if ret:
                frame = cv2.resize(frame, (640, 480))
                self.frame = frame
            else:
                self.cap.set(cv2.CAP_PROP_POS_FRAMES, 0)

    def read_frame(self):
        return self.frame

    def release(self):
        self.running = False
        self.thread.join()
        self.cap.release()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    estimator = DepthAndPoseEstimator()
    try:
        estimator.run()
    except Exception as e:
        print(f"An error occurred: {e}")

import os
import cv2
import torch
import numpy as np
from ultralytics import YOLO
import mediapipe as mp
from depth_anything_v2.dpt import DepthAnythingV2
from collections import deque
import asyncio
import concurrent.futures

os.environ["PYTORCH_CUDA_ALLOC_CONF"] = "expandable_segments:True"

class DepthAndPoseEstimator:
    def __init__(self, encoder='vits', video_source=0, moving_average_window_size=5):
        self.device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
        self.depth_model = self.initialize_depth_model(encoder)
        self.yolo_model = YOLO('yolov8s.pt', verbose=False)
        self.pose = mp.solutions.pose.Pose()
        self.cap = cv2.VideoCapture(video_source)
        self.depth_estimate = None
        self.saved_depth = None
        self.tracked_person_box = None  
        self.depth_history = deque(maxlen=moving_average_window_size)
        self.image_count = 0  # 이미지 저장 카운트

    def clear_memory(self):
        if self.device.type == 'cuda':
            torch.cuda.empty_cache()

    def initialize_depth_model(self, encoder):
        model_configs = {
            'vits': {'encoder': 'vits', 'features': 64, 'out_channels': [48, 96, 192, 384]}
        }
        depth_model = DepthAnythingV2(**model_configs[encoder])
        depth_model.load_state_dict(torch.load(f'./depth_anything_v2_{encoder}.pth', map_location='cpu'))
        return depth_model.to(self.device).eval()

    async def process_frame(self, raw_img):# detect_person에서 
        loop = asyncio.get_event_loop()
        person_box = await loop.run_in_executor(None, self.detect_person, raw_img)
        depth_scaled = None  

        if person_box:
            x1, y1, x2, y2 = person_box
            cropped_img = raw_img[:, x1:x2]  # Adjusted to capture the entire height
            #cv2.rectangle(raw_img, (x1, 0), (x2, raw_img.shape[0]), (0, 255, 0), 3)

            cropped_img = (cropped_img * 255).astype(np.uint8) if cropped_img.dtype != np.uint8 else cropped_img
            depth = await loop.run_in_executor(None, self.depth_model.infer_image, cropped_img)
            depth_normalized = (depth - depth.min()) / (depth.max() - depth.min() + 1e-5)
            depth_scaled = np.clip(depth_normalized * 50, 0, 255).astype(np.uint8)
            depth_scaled = cv2.GaussianBlur(depth_scaled, (5, 5), 0)
            self.tracked_person_box = person_box
            
            self.draw_person_box(raw_img, self.tracked_person_box, depth_scaled)

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

        if not self.tracked_person_box and current_boxes:
            self.tracked_person_box = current_boxes[0]  # Initialize first detected person

        if self.tracked_person_box and current_boxes:
            best_box, max_iou = None, 0
            for box in current_boxes:
                iou = self.calculate_iou(self.tracked_person_box, box)
                if iou > max_iou:
                    max_iou = iou
                    best_box = box

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
        if depth_scaled is None or person_box is None:
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

            if shoulder_middle is not None:
                left_shoulder_x = int(left_shoulder.x * w) if left_shoulder_visible else x1
                right_shoulder_x = int(right_shoulder.x * w) if right_shoulder_visible else x2

                cv2.rectangle(raw_img, (left_shoulder_x, 0), (right_shoulder_x, h), (0, 255, 0), 2)

                shoulder_y = (int(left_shoulder.y * h) + int(right_shoulder.y * h)) // 2
                cv2.circle(raw_img, (shoulder_middle, shoulder_y), 7, (0, 0, 255), -1)

                self.update_depth_estimate_from_circle(shoulder_middle, shoulder_y, depth_scaled, x1)
                self.display_depth_info(raw_img, shoulder_middle, shoulder_y)

                section_width = w // 3
                cv2.line(raw_img, (section_width, 0), (section_width, h), (255, 255, 255), 2)
                cv2.line(raw_img, (section_width * 2, 0), (section_width * 2, h), (255, 255, 255), 2)

                direction = 'Left' if shoulder_middle < section_width else 'Right' if shoulder_middle > section_width * 2 else ''
                if direction:
                    print("hello")
                    cv2.putText(raw_img, direction, (10, 120), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

    def calculate_shoulder_middle(self, left_shoulder, right_shoulder, width, x1, x2):
        if left_shoulder.visibility > 0.5 and right_shoulder.visibility > 0.5:
            left_shoulder_x = int(left_shoulder.x * width)
            right_shoulder_x = int(right_shoulder.x * width)
            return (left_shoulder_x + right_shoulder_x) // 2
        return None

    def update_depth_estimate_from_circle(self, shoulder_middle, shoulder_y, depth_scaled, x1):
        cropped_center_x = shoulder_middle - x1
        if 0 <= cropped_center_x < depth_scaled.shape[1] and 0 <= shoulder_y < depth_scaled.shape[0]:
            current_depth_value = np.mean(depth_scaled[shoulder_y, cropped_center_x])
            self.update_depth_estimate(current_depth_value)
            cv2.circle(depth_scaled, (cropped_center_x, shoulder_y), 7, (0, 0, 255), -1)

    def display_depth_info(self, raw_img, shoulder_middle, shoulder_y):
        if self.depth_estimate is not None:
            print("hello")
            cv2.putText(raw_img, f'Depth: {round(self.depth_estimate)}', (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255, 0, 0), 2)
            if self.depth_estimate > 34:
                print("hello")
                cv2.putText(raw_img, 'STRAIGHT', (10, 90), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            elif self.depth_estimate < 34:
                cv2.putText(raw_img, 'STOP', (10, 90), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)

        if self.saved_depth is not None:
            print("hello")
            #cv2.putText(raw_img, f'Saved Depth: {round(self.saved_depth)}', (10, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255, 0, 0), 2)
            if self.depth_estimate > 37:
                print("hello")
                cv2.putText(raw_img, 'STRAIGHT', (10, 90), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            elif self.depth_estimate < 35:
                cv2.putText(raw_img, 'STOP', (10, 90), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)

        #cv2.circle(raw_img, (shoulder_middle, shoulder_y), 5, (0, 0, 255), 2)

    def update_depth_estimate(self, current_depth_value):
        self.depth_estimate = current_depth_value if self.depth_estimate is None else (self.depth_estimate * 0.8) + (current_depth_value * 0.2)

    def save_depth(self, current_depth_value):
        self.saved_depth = current_depth_value

    async def run(self):
        if not self.cap.isOpened():
            print("Error: Could not open webcam.")
            return

        while True:
            ret, raw_img = self.cap.read()
            if not ret:
                print("Error: Failed to capture image.")
                break

            raw_img = cv2.flip(raw_img, 1)
            raw_img = cv2.resize(raw_img, (640, 480))

            processed_img, depth_image = await self.process_frame(raw_img)

            self.clear_memory()

            cv2.imshow('Original Image', processed_img)
            if depth_image is not None:
                depth_image = cv2.applyColorMap(depth_image.astype(np.uint8), cv2.COLORMAP_JET)
                cv2.imshow('Depth Image', depth_image)

            key = cv2.waitKey(1) & 0xFF
            if key == ord('q'):
                break
            elif key == ord('c'):
                if self.depth_estimate is not None:
                    self.save_depth(self.depth_estimate)
            elif key == ord('s'):  # 's' 키를 눌렀을 때 사진 저장
                self.image_count += 1
                image_filename = f'saved_image_{self.image_count}.png'
                cv2.imwrite(image_filename, processed_img)
                print(f'Saved image: {image_filename}')

        self.cap.release()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    asyncio.run(DepthAndPoseEstimator().run())

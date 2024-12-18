# 기본적인 얼굴 인식
import cv2
import mediapipe as mp

# MediaPipe 초기화
mp_face_mesh = mp.solutions.face_mesh
mp_drawing = mp.solutions.drawing_utils

face_mesh = mp_face_mesh.FaceMesh()
drawing_spec = mp_drawing.DrawingSpec(color=(0, 255, 0), thickness=1, circle_radius=1)

def draw_face_landmarks(frame, landmarks):
    # 얼굴 랜드마크를 프레임에 그립니다
    for landmark in landmarks:
        x = int(landmark.x * frame.shape[1])
        y = int(landmark.y * frame.shape[0])
        cv2.circle(frame, (x, y), 1, (0, 255, 0), -1)

def draw_connections(frame, landmarks):
    # 얼굴 랜드마크를 연결합니다
    connections = mp_face_mesh.FACEMESH_TESSELATION
    for connection in connections:
        start_idx, end_idx = connection
        start_landmark = landmarks[start_idx]
        end_landmark = landmarks[end_idx]
        start_point = (int(start_landmark.x * frame.shape[1]), int(start_landmark.y * frame.shape[0]))
        end_point = (int(end_landmark.x * frame.shape[1]), int(end_landmark.y * frame.shape[0]))
        cv2.line(frame, start_point, end_point, (0, 255, 0), 1)

# 웹캠 비디오 캡처
cap = cv2.VideoCapture(0)

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # 이미지 전처리
    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = face_mesh.process(frame_rgb)

    if results.multi_face_landmarks:
        for face_landmarks in results.multi_face_landmarks:
            draw_face_landmarks(frame, face_landmarks.landmark)
            draw_connections(frame, face_landmarks.landmark)

    # 화면에 비디오 표시
    cv2.imshow('Face Mesh', frame)

    # 종료 조건 (ESC 키를 누르면 종료)
    if cv2.waitKey(1) & 0xFF == 27:
        break

# 웹캠 해제 및 모든 창 닫기
cap.release()
cv2.destroyAllWindows()
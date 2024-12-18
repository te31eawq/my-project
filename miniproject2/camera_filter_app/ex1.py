import math
import sys
from typing import Tuple, Union

import cv2
import numpy as np
from mediapipe.python.solutions import drawing_utils, face_detection, face_mesh

# MediaPipe 초기화
mp_face_detection = face_detection
mp_face_mesh = face_mesh
mp_drawing = drawing_utils

TEXT_COLOR = (255, 0, 0)  # red

def _normalized_to_pixel_coordinates(
    normalized_x: float, normalized_y: float, image_width: int, image_height: int
) -> Union[None, Tuple[int, int]]:
    """Converts normalized value pair to pixel coordinates."""
    
    def is_valid_normalized_value(value: float) -> bool:
        return 0 <= value <= 1

    if not (
        is_valid_normalized_value(normalized_x)
        and is_valid_normalized_value(normalized_y)
    ):
        return None
    x_px = min(math.floor(normalized_x * image_width), image_width - 1)
    y_px = min(math.floor(normalized_y * image_height), image_height - 1)
    return x_px, y_px


def replace_background(image, face_landmarks, background_image):
    """Replaces the background with the specified image while keeping the face black."""
    height, width, _ = image.shape

    # 얼굴 윤곽선과 이마를 포함한 MediaPipe 인덱스
    face_landmarks_indices = [
        10, 338, 297, 332, 284, 251, 389, 356, 454, 323, 361, 288, 397, 365, 379, 378,
        400, 377, 152, 148, 176, 149, 150, 136, 172, 58, 132, 93, 234, 127, 162, 21, 
        54, 103, 67, 109
    ]

    # 얼굴 윤곽선 점들을 Numpy 배열로 변환
    points = np.array(
        [
            (
                int(face_landmarks.landmark[idx].x * width),
                int(face_landmarks.landmark[idx].y * height),
            )
            for idx in face_landmarks_indices
        ],
        np.int32,
    )

    # 얼굴 영역을 위한 마스크 생성
    mask = np.zeros(image.shape[:2], dtype=np.uint8)
    cv2.fillPoly(mask, [points], (255, 255, 255))

    # 배경 이미지를 원본 이미지 크기로 조정
    resized_background = cv2.resize(background_image, (width, height))

    # 얼굴 부분을 검정색으로 설정
    face_black = np.zeros_like(image)
    face_black = cv2.bitwise_and(face_black, face_black, mask=mask)

    # 배경만 남기고 나머지 부분을 배경 이미지로 채움
    inverse_mask = cv2.bitwise_not(mask)
    background_with_face_black = cv2.bitwise_and(resized_background, resized_background, mask=inverse_mask)

    # 얼굴을 검정색으로 하고, 배경을 합성
    combined_image = cv2.add(face_black, background_with_face_black)
    
    return combined_image


if __name__ == "__main__":
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("Error: Could not open camera.")
        sys.exit()

    # 배경 이미지 로드
    background_image = cv2.imread('ocean.jpg')

    mode = 0  # 0: No Filter, 1: Face Detection, 2: Face Mesh, 3: Replace Background

    with mp_face_detection.FaceDetection(
        model_selection=0, min_detection_confidence=0.5
    ) as face_detection, mp_face_mesh.FaceMesh(
        static_image_mode=False,
        max_num_faces=1,
        refine_landmarks=True,
        min_detection_confidence=0.5,
        min_tracking_confidence=0.5,
    ) as face_mesh_detector:
        while cap.isOpened():
            ret, frame = cap.read()
            if not ret:
                print("Ignoring empty camera frame.")
                continue

            # Q 키를 누르면 No Filter -> Face Detection -> Face Mesh -> Replace Background 전환
            if cv2.waitKey(5) & 0xFF == ord("q"):
                mode = (mode + 1) % 4  # 모드를 0, 1, 2, 3으로 전환
                print(
                    f"{['No Filter', 'Face Detection', 'Face Mesh', 'Replace Background'][mode]} Enabled"
                )

            if mode == 3:
                # Replace Background 모드: 얼굴은 검정색, 배경은 ocean.jpg로 교체
                image = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                results = face_mesh_detector.process(image)
                image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)

                if results.multi_face_landmarks:
                    for face_landmarks in results.multi_face_landmarks:
                        combined_image = replace_background(image, face_landmarks, background_image)
                        cv2.imshow("Replace Background", combined_image)
                else:
                    cv2.imshow("Replace Background", image)

            else:
                # No Filter 모드 (원본 이미지 출력)
                cv2.imshow("Face Detection", frame)

            # ESC 키로 종료
            if cv2.waitKey(5) & 0xFF == 27:
                break

    cap.release()
    cv2.destroyAllWindows()

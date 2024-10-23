import RPi.GPIO as GPIO
import bluetooth
import time

# GPIO 모터 핀 정의
ENA = 12  # 오른쪽 모터 속도 제어 핀
IN1 = 8   # 오른쪽 모터 방향 제어 핀
IN2 = 9   # 오른쪽 모터 방향 제어 핀
ENB = 13  # 왼쪽 모터 속도 제어 핀
IN3 = 6   # 왼쪽 모터 방향 제어 핀
IN4 = 7   # 왼쪽 모터 방향 제어 핀

# PWM 설정값
forward_backward_pwm = 50  # 전진 및 후진 시 PWM 값
turn_pwm = 50              # 회전 시 PWM 값

def setup_gpio():
    # GPIO 설정
    GPIO.setmode(GPIO.BCM)  # BCM 모드 설정
    GPIO.setup(ENA, GPIO.OUT)
    GPIO.setup(IN1, GPIO.OUT)
    GPIO.setup(IN2, GPIO.OUT)
    GPIO.setup(ENB, GPIO.OUT)
    GPIO.setup(IN3, GPIO.OUT)
    GPIO.setup(IN4, GPIO.OUT)

    # PWM 설정 (100Hz 주파수)
    global pwmA, pwmB
    if 'pwmA' not in globals():
        pwmA = GPIO.PWM(ENA, 100)  # 100Hz 주파수
    if 'pwmB' not in globals():
        pwmB = GPIO.PWM(ENB, 100)  # 100Hz 주파수
    pwmA.start(0)
    pwmB.start(0)

def forward():
    GPIO.output(IN1, GPIO.HIGH)
    GPIO.output(IN2, GPIO.LOW)
    GPIO.output(IN3, GPIO.LOW)
    GPIO.output(IN4, GPIO.HIGH)
    pwmA.ChangeDutyCycle(forward_backward_pwm)
    pwmB.ChangeDutyCycle(forward_backward_pwm)

def backward():
    GPIO.output(IN1, GPIO.LOW)
    GPIO.output(IN2, GPIO.HIGH)
    GPIO.output(IN3, GPIO.HIGH)
    GPIO.output(IN4, GPIO.LOW)
    pwmA.ChangeDutyCycle(forward_backward_pwm)
    pwmB.ChangeDutyCycle(forward_backward_pwm)

def turn_right():
    GPIO.output(IN1, GPIO.LOW)
    GPIO.output(IN2, GPIO.HIGH)
    GPIO.output(IN3, GPIO.LOW)
    GPIO.output(IN4, GPIO.HIGH)
    pwmA.ChangeDutyCycle(turn_pwm)
    pwmB.ChangeDutyCycle(turn_pwm)

def turn_left():
    GPIO.output(IN1, GPIO.HIGH)
    GPIO.output(IN2, GPIO.LOW)
    GPIO.output(IN3, GPIO.HIGH)
    GPIO.output(IN4, GPIO.LOW)
    pwmA.ChangeDutyCycle(turn_pwm)
    pwmB.ChangeDutyCycle(turn_pwm)

def stop_motors():
    pwmA.ChangeDutyCycle(0)
    pwmB.ChangeDutyCycle(0)
    GPIO.output(IN1, GPIO.LOW)
    GPIO.output(IN2, GPIO.LOW)
    GPIO.output(IN3, GPIO.LOW)
    GPIO.output(IN4, GPIO.LOW)

def bluetooth_setup():
    # 블루투스 소켓 설정
    server_sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
    server_sock.bind(("", bluetooth.PORT_ANY))  # 사용 가능한 포트에 바인딩
    server_sock.listen(1)
    return server_sock

def cleanup_gpio():
    if GPIO.getmode() is not None:  # GPIO가 설정된 경우만 정리
        GPIO.cleanup()

def main():
    server_sock = None
    while True:  # 무한 루프로 재연결 시도
        try:
            setup_gpio()  # GPIO 재설정
            if server_sock is None:
                server_sock = bluetooth_setup()
            print("Waiting for connection on RFCOMM channel 1")

            client_sock, client_info = server_sock.accept()
            print(f"Accepted connection from {client_info}")

            # 블루투스 데이터 수신
            while True:
                try:
                    data = client_sock.recv(1024).decode('utf-8')
                    if len(data) == 0:  # 클라이언트 연결 종료 시
                        print("Client disconnected")
                        stop_motors()
                        break  # 내부 루프 종료, 새로운 연결 대기
                    print(f"Received: {data}")

                    if data == 'F':  # 전진
                        forward()
                    elif data == 'B':  # 후진
                        backward()
                    elif data == 'R':  # 오른쪽 회전
                        turn_right()
                    elif data == 'L':  # 왼쪽 회전
                        turn_left()
                    elif data in ['f', 'b', 'r', 'l']:  # 정지
                        stop_motors()
                except bluetooth.BluetoothError:
                    print("Bluetooth Error occurred")
                    stop_motors()
                    break  # 예외 발생 시 내부 루프 종료

            # 연결 종료 후 소켓 닫기
            client_sock.close()

        except (OSError, KeyboardInterrupt):
            print("Program interrupted or error occurred")
            stop_motors()
            cleanup_gpio()
            break  # 외부 루프 종료, 프로그램 종료

        finally:
            cleanup_gpio()  # GPIO 정리

if __name__ == "__main__":
    main()
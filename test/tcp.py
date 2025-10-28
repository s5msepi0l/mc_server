import socket
import time

HOST = '127.0.0.1'  # localhost
PORT = 25565        # must match your C++ server PORT

def main():
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, PORT))
            print(f"Connected to server at {HOST}:{PORT}")

            # Send test messages
            for i in range(5):
                message = f"Hello server! Message {i}\n"
                s.sendall(message.encode('utf-8'))
                print(f"Sent: {message.strip()}")
                time.sleep(1)

            print("Closing connection")
    except ConnectionRefusedError:
        print(f"Could not connect to server at {HOST}:{PORT}. Is it running?")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()

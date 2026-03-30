import socket
import matplotlib.pyplot as plt
from collections import deque

# --- 1. 設定區 ---
HOST = '0.0.0.0'  # 監聽所有網路介面，解決 10049 錯誤
PORT = 8002       # 必須與 STM32 程式碼中的 RemotePORT 一致
MAX_POINTS = 100  # 圖面上顯示的資料點數 (越小反應越快)

# 資料緩衝區 (使用 deque 自動滾動數據)
x_history = deque([0] * MAX_POINTS, maxlen=MAX_POINTS)
y_history = deque([0] * MAX_POINTS, maxlen=MAX_POINTS)
z_history = deque([0] * MAX_POINTS, maxlen=MAX_POINTS)

def start_visualizer():
    # 建立 TCP Socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # 允許重新使用位址，防止重啟 Server 時報錯
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST, PORT))
        s.listen()
        
        print(f"=== TCP Server 已啟動 (Port: {PORT}) ===")
        print("等待 STM32 連線中...")
        
        conn, addr = s.accept() # 程式會在這裡停住，直到板子連上
        with conn:
            print(f"\n✅ 成功連線！來源設備: {addr}")
            
            # --- 2. 連線成功後才初始化繪圖視窗 ---
            plt.ion() # 開啟互動模式
            fig, ax = plt.subplots(figsize=(10, 6))
            line_x, = ax.plot(list(x_history), 'r-', label='X-axis (mg)')
            line_y, = ax.plot(list(y_history), 'g-', label='Y-axis (mg)')
            line_z, = ax.plot(list(z_history), 'b-', label='Z-axis (mg)')
            
            ax.set_ylim(-2000, 2000) # 加速度計範圍 (可視需求調整)
            ax.set_title("STM32 Real-time Accelerometer Data")
            ax.set_xlabel("Time Samples")
            ax.set_ylabel("Acceleration (mg)")
            ax.legend(loc='upper right')
            ax.grid(True)

            print("開始接收數據... (繪圖視窗已開啟)")
            
            while True:
                try:
                    # 接收數據 (1024 bytes)
                    raw_data = conn.recv(1024).decode('utf-8').strip()
                    if not raw_data:
                        print("\n[提示] STM32 已中斷連線。")
                        break
                    
                    # 處理加分題：重大動作偵測警報
                    if "EVENT:MOTION" in raw_data:
                        print("\nSignificant Motion！")
                        continue

                    # 解析 CSV 格式數據 (例如: "12, -45, 1002")
                    parts = raw_data.split(',')
                    if len(parts) == 3:
                        x, y, z = map(int, parts)
                        
                        # 更新數據
                        x_history.append(x)
                        y_history.append(y)
                        z_history.append(z)
                        
                        # 更新線段圖表
                        line_x.set_ydata(list(x_history))
                        line_y.set_ydata(list(y_history))
                        line_z.set_ydata(list(z_history))
                        
                        # [關鍵] 讓 GUI 有時間重繪，才不會卡死變白畫面
                        plt.pause(0.001) 
                        
                        # 在終端機顯示目前數值 (使用 \r 讓它維持在同一行)
                        print(f" 接收中 -> X:{x:5} | Y:{y:5} | Z:{z:5}", end='\r')

                except Exception as e:
                    print(f"\n[錯誤] 發生異常: {e}")
                    break

    plt.ioff()
    plt.show()

if __name__ == "__main__":
    try:
        start_visualizer()
    except KeyboardInterrupt:
        print("\n使用者手動停止程式。")
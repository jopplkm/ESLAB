from bluepy.btle import Peripheral, UUID, DefaultDelegate, Scanner
import time

# ==========================================
# 1. 定義掃描時用到的 Delegate (信箱)
# ==========================================
class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        # 這裡我們保持安靜，等掃描完再一次印出清單，畫面比較乾淨
        pass 

# ==========================================
# 2. 定義連線後用來接收通知的 Delegate (信箱)
# ==========================================
class MyDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleNotification(self, cHandle, data):
        print(f"\n\n[🔔 叮咚！收到來自手機的通知！]")
        print(f"👉 原始 Hex 資料: {data.hex()}")
        try:
            print(f"👉 解碼成文字: {data.decode('utf-8')}")
        except:
            print("(無法解碼成一般文字)")
        print("\n⏳ 繼續等待下一個通知... (按 Ctrl+C 結束)")

# ==========================================
# 主程式開始
# ==========================================
SVC_UUID = "0000fff0-0000-1000-8000-00805f9b34fb"
CHAR_UUID = "0000fff0-0000-1000-8000-00805f9b34fb"

try:
    # --- 階段一：掃描附近的裝置 ---
    print("🔍 開始掃描附近的藍牙裝置 (請稍候 10 秒)...")
    scanner = Scanner().withDelegate(ScanDelegate())
    devices = scanner.scan(10.0)

    n = 0
    addr_list = []
    print("\n--- 掃描結果 ---")
    for dev in devices:
        # 使用 getValueText(9) 來抓取 Complete Local Name (裝置完整名稱)
        dev_name = dev.getValueText(9)
        
        # 如果抓不到完整名稱，試著抓 Shortened Local Name (簡稱，代號 8)
        if not dev_name:
            dev_name = dev.getValueText(8)
            
        # 如果還是沒有名字，就顯示 Unknown
        if not dev_name:
            dev_name = "Unknown"

        print(f"[{n}] MAC: {dev.addr} | 名稱: {dev_name} | 訊號: {dev.rssi} dB")
        addr_list.append(dev.addr)
        n += 1

    if len(addr_list) == 0:
        print("❌ 找不到任何裝置！請確認手機廣播已開啟。")
        exit()

    # --- 階段二：讓使用者選擇 ---
    number = input('\n👉 請輸入你要連線的手機裝置編號 (例如 0, 1, 2...): ')
    target_mac = addr_list[int(number)]
    print(f"\n準備連線至手機: {target_mac}")

    # --- 階段三：連線與寫入 CCCD ---
    print("連線中...")
    dev = Peripheral(target_mac, 'random')
    
    # 綁定接收通知的信箱
    dev.setDelegate(MyDelegate())
    print("連線成功！\n")

    service = dev.getServiceByUUID(UUID(SVC_UUID))
    characteristic = service.getCharacteristics(UUID(CHAR_UUID))[0]
    cccd_desc = characteristic.getDescriptors(forUUID=UUID(0x2902))[0]

    if cccd_desc:
        print("找到 CCCD，準備寫入 0x0002 訂閱 Indications...")
        cccd_desc.write(b"\x02\x00", withResponse=True)
        print("✅ 訂閱成功！\n")
        
        # --- 階段四：進入無限監聽模式 ---
        print("==================================================")
        print("🎧 RPi 正在監聽中... 請現在去手機端更改 Value！")
        print("==================================================")
        
        while True:
            # 等待手機傳來的通知，超時設為 1 秒
            if dev.waitForNotifications(1.0):
                continue
            # 畫面上印點點點，表示程式還在活著監聽中
            print(".", end="", flush=True)

    else:
        print("找不到 CCCD。")

except KeyboardInterrupt:
    print("\n\n🛑 收到中斷指令，準備關閉連線。")
except Exception as e:
    print(f"\n❌ 發生錯誤: {e}")
    print("如果看到 code: 13 錯誤，請先執行 sudo hciconfig hci0 reset 重置網卡")
finally:
    if 'dev' in locals():
        dev.disconnect()
        print("已安全斷開連線，程式結束。")
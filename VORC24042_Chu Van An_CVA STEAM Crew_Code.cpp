//Khai báo các thư viện cơ bản của C++
#include <stdio.h> 
#include <math.h>

//Khai báo thư viện Arduino PCA9685
#include <Wire.h> //thư viện I2c của Arduino, do PCA9685 sử dụng chuẩn giao tiếp i2c nên thư viện này bắt buộc phải khai báo 
#include <Adafruit_PWMServoDriver.h> // thư viện PCA9685

//Xác định các chân động cơ:
//Đối với các motor không phải là servo cần 2 cổng PCA9685

//Cơ cấu đẩy
#define daytrai 2 
#define dayphai 3 

//Cơ cấu nâng
#define nang1 8 
#define nang0 9

//Cơ cấu thu
#define lay1 10 
#define lay0 11

//Cơ cấu di chuyển
#define dcphai1 12 
#define dcphai0 13
#define dctrai1 14 
#define dctrai0 15

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(); //Khởi tạo class của thư viện với địa chỉ gốc

//Khai báo thư viện cho tay cầm
#include <PS2X_lib.h> // Khai báo thư viện

//Xác định các chân điều khiển 
#define PS2_DAT 12 // MISO 
#define PS2_CMD 13 // MOSI 
#define PS2_SEL 15 // SS 
#define PS2_CLK 14 // SLK

//Khởi tạo class của thư viện
PS2X ps2x; // khởi tạo class PS2x

void setupcontrol()
{
    //Khởi tạo Serial monitor với tốc độ 115200
    Serial.begin(115200);
    Serial.print("Ket noi voi tay cam PS2:");

    //Kết nối với tay cầm bằng hàm ps2x.config_gamepad, thử kết nối lại trong vòng 3 lần nếu quá 3 lần không kết nối được với tay cầm thì sẽ dừng lại

    int error = -1; 
    for (int i = 0; i < 3; i++) // thử kết nối với tay cầm ps2 trong 3 lần 
    {
        delay(1000); // đợi 1 giây 
        // cài đặt chân và các chế độ: GamePad
        error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble); 
        Serial.print("x"); 
        if(!error) //kiểm tra nếu tay cầm đã kết nối thành công
            Serial.print("v") 
            break; // thoát khỏi vòng lặp 
    } 
}

void loop()
{
    ps2x.read_gamepad(false, false); // gọi hàm để đọc tay điều khiển

    // các trả về giá trị TRUE khi nút được giữ
    if (ps2x.Button(PSB_START)) // nếu nút Start được giữ, in ra Serial monitor
        Serial.println("Start dang duoc bam");
    if (ps2x.Button(PSB_SELECT)) // nếu nút Select được giữ, in ra Serial monitor
        Serial.println("Select dang duoc bam");
    
    // Trả về giá trị TRUE khi các nút o, x được thay đổi trạng thái
    if (ps2x.NewButtonState(PSB_CIRCLE))
        Serial.println("o vua duoc thay doi trang thai");
    
    if (ps2x.NewButtonState())
    { // Trả về giá trị TRUE khi nút được bấm
    if (ps2x.Button(PSB_L1))
        Serial.println("L1 duoc bam");
    if (ps2x.Button(PSB_L2))
        Serial.println("L2 duoc bam");
    if (ps2x.Button(PSB_R1))
        Serial.println("R1 duoc bam");
    if (ps2x.Button(PSB_R2))
        Serial.println("R2 duoc bam");
    }
    if (ps2x.Button(PSB_CROSS)) //Giữ nút X để đọc giá trị joystick
    {
        Serial.print("Y num trai: ");
        Serial.print(ps2x.Analog(PSS_LY), DEC); // đọc trục Y của joystick bên trái
        Serial.print("Y num phai: ");
        Serial.print(ps2x.Analog(PSS_RY), DEC); // đọc trục Y của joystick bên phải
    }
}  

void setupmotor()
{
    pwm.begin(); //khởi tạo PCA9685
    pwm.setOscillatorFrequency(27000000); // cài đặt tần số dao động
    pwm.setPWMFreq(50);// cài đặt tần số PWM
    Wire.setClock(400000); // cài đặt tốc độ giao tiếp i2c ở tốc độ cao nhất(400 Mhz)

}
void code()
{
    while (true) // Luôn cập nhật giá trị các nút điều khiến để truyền tương ứng cho các motor
    {
        y1=ps2x.Analog(PSS_RY)/2; // Gán y1 là nửa giá trị của joystick phải trên trục tung
        pwm.setPWM(dcphai1, 0, ((y1-fabs(y1))*4096)/100); // % Xung pwm (4096) tương ứng với độ lớn từ 0-100 của joystick phải trên trục tung
        pwm.setPWM(dcphai0, 0, ((y1+fabs(y1))*4096)/100); // % Xung pwm (4096) tương ứng với độ lớn 0-100 của joystick phải trên trục tung
        // y1 âm thì dcphai1 có toạ độ tắt dương, dcphai0 có toạ độ tắt là 0 và ngược lại. Bởi vì makerbot muốn chạy thì 1 trong 2 chân PCA9685 phải ở trạng thái tắt
        // Khi y1 dương thì bánh phải lùi, y1 âm thì bánh trái tiến
        
        // Tương tự như trên với bánh phải
        y2=ps2x.Analog(PSS_LY)/2;
        pwm.setPWM(dctrai1, 0, ((y2+fabs(y2))*2048)/100); 
        pwm.setPWM(dctrai0, 0, ((y2-fabs(y2))*2048)/100); // Ngược lại với bánh trái, 2 motor phải chạy ngược chiều mới có thể di chuyển đúng theo điều khiển
        // Khi y2 dương thì bánh phải tiến, y2 âm thì bánh phải lùi
        
        // Xung pwm motor của cơ cấu thu bóng tương ứng với nút PSB_L1 và PSB_L2, tốc độ 50% (2048/4096)
        pwm.setPWM(lay1, 0, ps2x.Button(PSB_L1)*2048); 
        pwm.setPWM(lay0, 0, ps2x.Button(PSB_L2)*2048);

        // Xung pwm motor của cơ cấu nâng tương ứng với nút PSB_R1 và PSB_R2, tốc độ 25% (1024/4096)
        pwm.setPWM(nang1, 0, ps2x.Button(PSB_R1)*1024); 
        pwm.setPWM(nang0, 0, ps2x.Button(PSB_R2)*1024);

        // Xung pwm motor của cơ cấu đẩy tương ứng với nút PSB_CIRCLE, tốc độ 25% (1024/4096)
        pwm.setPWM(dayphai, 0, ps2x.Button(PSB_CIRCLE)*1024);
        pwm.setPWM(daytrai, 0, ps2x.Button(PSB_CIRCLE)*1024);

    }
}
int main // Chạy các chương trình
{
    setupcontrol();
    setupmotor();
    loop();
    code();
}

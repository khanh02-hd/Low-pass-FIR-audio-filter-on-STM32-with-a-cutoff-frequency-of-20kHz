clc; clear; close all;

% ===== 1) CẤU HÌNH =====
COM = "COM8";           
baud = 115200;
fs = 48000;
FFT_SIZE = 256;

% ===== 2) TÁI TẠO TÍN HIỆU GỐC (ĐỂ SO SÁNH) =====
n = 0:FFT_SIZE-1;
% Tín hiệu lý thuyết: 3kHz + 23kHz
x_ref = sin(2*pi*3000*n/fs) + sin(2*pi*23000*n/fs);
x_ref = x_ref / max(abs(x_ref)); % chuẩn hóa biên độ về 1

% ===== 3) MỞ SERIAL & NHẬN DỮ LIỆU TỪ STM32 =====
disp("-------------------------------------------------");
disp("BẮT ĐẦU CHẾ ĐỘ THU DỮ LIỆU (MODE A)");
disp("Bước 1: Matlab đang chờ kết nối...");
disp("Bước 2: >>> HÃY BẤM NÚT RESET (MÀU ĐEN) TRÊN STM32 NGAY! <<<");
disp("-------------------------------------------------");

try
    s = serialport(COM, baud);
    configureTerminator(s, "LF");
    s.Timeout = 15; % Tăng thời gian chờ lên 15s cho bạn kịp bấm nút
    
    flush(s); % Xóa sạch dữ liệu rác trong buffer trước khi nhận
    
    y_stm32 = zeros(1, FFT_SIZE);
    
    % Đọc 256 mẫu
    for i = 1:FFT_SIZE
        data_str = readline(s);
        
        % Kiểm tra nếu nhận được chuỗi rỗng (do timeout hoặc lỗi)
        if isempty(data_str)
            error("Không nhận được dữ liệu! (Quá thời gian chờ hoặc chưa bấm Reset)");
        end
        
        y_stm32(i) = str2double(data_str);
    end
    
    clear s; % Đóng cổng COM ngay sau khi nhận xong để tránh kẹt
    disp(">>> Đã nhận đủ 256 mẫu! Đang vẽ đồ thị...");
    
catch e
    clear s; % Đảm bảo đóng cổng COM nếu có lỗi
    error("LỖI KẾT NỐI: " + e.message + " (Kiểm tra lại tên cổng COM hoặc dây cáp)");
end

% ===== 4) TÍNH TOÁN FFT =====
% (A) FFT tín hiệu gốc (Lý thuyết)
X = fft(x_ref);
P2 = abs(X/FFT_SIZE);
P1_ref = P2(1:FFT_SIZE/2+1);
P1_ref(2:end-1) = 2*P1_ref(2:end-1);

% (B) FFT tín hiệu thực tế (Từ STM32)
Y = fft(y_stm32);
P2 = abs(Y/FFT_SIZE);
P1_stm = P2(1:FFT_SIZE/2+1);
P1_stm(2:end-1) = 2*P1_stm(2:end-1);

f = fs*(0:(FFT_SIZE/2))/FFT_SIZE; % Trục tần số

% ===== 5) VẼ SO SÁNH =====
figure('Name','MODE A - TEST: Time & FFT', 'Color', 'w');

% Đồ thị 1: Thời gian - Gốc
subplot(2,2,1);
plot(n/fs, x_ref, 'r', 'LineWidth', 1); grid on;
title('Time: Tín hiệu Gốc (Simulated)');
xlabel('Time (s)'); ylabel('Amplitude');
ylim([-1.5 1.5]);

% Đồ thị 2: Thời gian - STM32
subplot(2,2,2);
plot(n/fs, y_stm32, 'b', 'LineWidth', 1); grid on;
title('Time: Tín hiệu sau lọc FIR (STM32)');
xlabel('Time (s)'); ylabel('Amplitude');
ylim([-1.5 1.5]);

% Đồ thị 3: Phổ tần - Gốc
subplot(2,2,3);
plot(f, P1_ref, 'r', 'LineWidth', 1.5); grid on;
title('FFT: Tín hiệu Gốc (Có nhiễu 23kHz)');
xlabel('Frequency (Hz)'); ylabel('Magnitude');
xlim([0 24000]);

% Đồ thị 4: Phổ tần - STM32
subplot(2,2,4);
plot(f, P1_stm, 'b', 'LineWidth', 1.5); grid on;
title('FFT: Kết quả thực tế STM32 (Đã lọc)');
xlabel('Frequency (Hz)'); ylabel('Magnitude');
xlim([0 24000]);

sgtitle('SO SÁNH KẾT QUẢ MÔ PHỎNG VÀ THỰC TẾ TRÊN STM32');

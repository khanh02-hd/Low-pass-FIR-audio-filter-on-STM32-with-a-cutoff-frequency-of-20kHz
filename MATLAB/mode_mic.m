clc; clear; close all;

%% ===== 1. CẤU HÌNH =====
COM = "COM9";           
baud = 115200;
fs = 48000;             % Tần số lấy mẫu
N = 512;                % Kích thước gói

disp("-------------------------------------------------");
disp("MODE: 3-VIEW MONITOR (HAMMING WINDOW VERSION)");
disp(">>> Hãy Alo hoặc Huýt sáo <<<");
disp("Bấm Ctrl+C trên Command Window để dừng.");
disp("-------------------------------------------------");

try
    s = serialport(COM, baud);
    configureTerminator(s, "LF");
    s.Timeout = 2; 
    flush(s);      
catch e
    error("Lỗi kết nối COM: " + e.message);
end

%% ===== 2. KHỞI TẠO XỬ LÝ SỐ (DSP) =====

w = hamming(N)'; 
win_correction = 2 / sum(w); 
f = fs*(0:(N/2))/N;

%% ===== 3. TẠO ĐỒ THỊ =====
figure('Name','REAL-TIME','Color','w', 'Position', [100 100 800 900]);

% --- ĐỒ THỊ 1: Miền thời gian ---
subplot(3,1,1);
h_time_raw = plot(zeros(1,N), 'r', 'Color', [1 0.7 0.7]); hold on; 
h_time_fir = plot(zeros(1,N), 'b', 'LineWidth', 1);               
hold off; grid on;
title('1. Miền thời gian (Time Domain)');
ylabel('Amplitude'); ylim([-1.2 1.2]); xlim([1 N]);
legend('Raw Input', 'FIR Output');

% --- ĐỒ THỊ 2: Phổ toàn dải (0 - 24 kHz) ---
subplot(3,1,2);
h_fft_full_raw = plot(f, zeros(1,N/2+1), 'r', 'LineWidth', 1); hold on;
h_fft_full_fir = plot(f, zeros(1,N/2+1), 'b', 'LineWidth', 1.5);
hold off; grid on;
title('2. Phổ toàn dải (0 - 24 kHz) - Kiểm tra lọc nhiễu');
ylabel('Magnitude');
xlim([0 24000]); ylim([0 0.15]); 
legend('Raw', 'Filtered');

% --- ĐỒ THỊ 3: Phổ Zoom (0 - 2 kHz) ---
subplot(3,1,3);
h_fft_zoom_raw = plot(f, zeros(1,N/2+1), 'r', 'LineWidth', 1); hold on;
h_fft_zoom_fir = plot(f, zeros(1,N/2+1), 'b', 'LineWidth', 1.5);
hold off; grid on;
title('3. Phổ chi tiết (Zoom 0 - 2 kHz)');
xlabel('Frequency (Hz)'); ylabel('Magnitude');
xlim([20 2000]);      
ylim('auto');         % Auto scale để thấy rõ tín hiệu nhỏ
legend('Raw', 'Filtered');

%% ===== 4. VÒNG LẶP VÔ TẬN =====
raw_buf = zeros(1, N);
fir_buf = zeros(1, N);

try
    while true
        % Xả bộ đệm để Realtime
        flush(s); 
        
        % Đọc dữ liệu
        cnt = 0;
        while cnt < N
            line = readline(s);
            try
                v = sscanf(line, "%f,%f");
                if numel(v) == 2
                    cnt = cnt + 1;
                    raw_buf(cnt) = v(1);
                    fir_buf(cnt) = v(2);
                end
            catch
                continue;
            end
        end

        % --- Tính toán FFT ---
        Y_raw = fft(raw_buf .* w);       
        P1_raw = abs(Y_raw(1:N/2+1)) * win_correction;
        
        Y_fir = fft(fir_buf .* w);
        P1_fir = abs(Y_fir(1:N/2+1)) * win_correction;

        % --- Cập nhật ĐỒNG THỜI 3 đồ thị ---
        
        % 1. Time Domain
        set(h_time_raw, 'YData', raw_buf);
        set(h_time_fir, 'YData', fir_buf);
        
        % 2. Full FFT 
        set(h_fft_full_raw, 'YData', P1_raw);
        set(h_fft_full_fir, 'YData', P1_fir);
        
        % 3. Zoom FFT 
        set(h_fft_zoom_raw, 'YData', P1_raw);
        set(h_fft_zoom_fir, 'YData', P1_fir);
        
        drawnow limitrate; 
    end
catch e
    clear s;
    disp("Đã dừng chương trình.");
end
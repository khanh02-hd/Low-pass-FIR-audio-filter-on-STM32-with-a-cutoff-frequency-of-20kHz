clc; clear;

fs = 48000;
FFT_SIZE = 256;
n = 0:FFT_SIZE-1;

% Tín hiệu trong băng + ngoài băng
x_test = sin(2*pi*3000*n/fs) + sin(2*pi*23000*n/fs);

% Chuẩn hóa biên độ
x_test = x_test / max(abs(x_test));

% OUTPUT 1: đồ thị thời gian
figure;
plot(n/fs, x_test);
title('Tín hiệu kiểm chứng: 3 kHz + 23 kHz');
xlabel('Time (s)');
ylabel('Amplitude');

% OUTPUT 2: in ra mảng C (ĐỂ COPY)
fprintf('float test_signal[%d] = {\n', FFT_SIZE);
fprintf('%ff,\n', x_test);
fprintf('};\n');

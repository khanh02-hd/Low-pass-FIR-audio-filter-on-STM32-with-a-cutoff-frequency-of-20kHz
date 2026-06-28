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
fc = 20000;
N  = 51;
h = fir1(N-1, fc/(fs/2), 'low', hamming(N));

figure;
freqz(h,1,1024,fs);
title('Đáp ứng tần số bộ lọc FIR');
fprintf('float fir_coeff[%d] = {\n', N);
fprintf('%ff,\n', h);
fprintf('};\n');
y = filter(h,1,x_test);

figure;
subplot(2,1,1);
pwelch(x_test,[],[],[],fs);
title('FFT trước lọc');

subplot(2,1,2);
pwelch(y,[],[],[],fs);
title('FFT sau lọc FIR');


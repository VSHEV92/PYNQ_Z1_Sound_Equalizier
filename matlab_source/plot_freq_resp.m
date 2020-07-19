[hl,w] = freqz(lpf,1,512);
[hb,w] = freqz(bpf,1,512);
[hh,w] = freqz(hpf,1,512);

plot(abs(hl+hb+hh))

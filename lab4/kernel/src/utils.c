typedef unsigned long long int uint64_t;
typedef unsigned char uint8_t;

uint8_t hex_to_int8(char hex){
    if(hex >= '0' && hex <= '9')
        return hex-'0';
    else if(hex >= 'A' && hex <= 'Z')
        return hex-'A'+10;
    else if(hex >= 'a' && hex <= 'z')
        return hex-'a'+10;
    else
        return -1;
}

uint64_t hex_to_int64(char* num){
    uint64_t res=0;
    for(int i=0; i<8; i++){
        res = (res<<4) + hex_to_int8(num[i]);
    }
    return res;
}

uint64_t log2(uint64_t num) {
  for (uint64_t i = 0; i < 64; i++) {
    if (num == (1 << i)) return i;
  }
  return 0;
}
uint64_t align_up(uint64_t addr, uint64_t alignment) {
  return (addr + alignment - 1) & (~(alignment - 1));
}

uint64_t align_up_exp(uint64_t n) {
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n |= n >> 32;
  n++;
  return n;
}
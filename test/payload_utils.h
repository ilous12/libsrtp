#ifndef WEBRTC_MODULES_AUDIO_CODING_CODECS_AMRWB_PAYLOAD_UTIL_H_
#define WEBRTC_MODULES_AUDIO_CODING_CODECS_AMRWB_PAYLOAD_UTIL_H_

#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
  const uint8_t* start;
  uint8_t* current;
  const uint8_t* end;
  uint8_t remain_bit;
  uint16_t size;
} payload_t;

/**
  * 전달받은 pointer 및 length를 매개로 payload_t를 생성하여 돌려줌.
  */
static payload_t* payload_create(const uint8_t* pointer, int16_t length, uint8_t initialize);
/**
  * payload의 current pointer가 끝까지 이동했는지 여부를 돌려줌.
  */
static uint8_t payload_eof(payload_t* payload);
/**
  * payload에 남아 있는 데이터에서 파라메터로 전달받은 length만큼 읽어들여 돌려줌.
  * return 읽어 들인 bit값
  */
static uint8_t payload_read_bit(payload_t* payload, int16_t bit_length);
/**
  * payload에 남아 있는 데이터를 읽어들여 돌려줌.
  * payload에 남아 있는 데이터가 byte align이 맞지 않을 경우 
  * 남아있는 bit부터 읽어 들여 byte데이터로 만들어  돌려줌.
  * Ex) 110 11001100이 남아 있을 경우 11011001 10000000( 나머지 bit는 0으로 채움)
  * return 읽어 들인 byte수
  */
static uint8_t payload_read_bytes(payload_t* payload, uint8_t* out, int16_t bit_length);
/**
  * 완전한 바이트 단위로 데이터를 읽어 들임.
  */
static int16_t payload_read_bytes_ot(payload_t* payload, uint8_t* out, int16_t length);
/**
  * payload에서 남아있는 최상위 bit를 읽어들려 돌려줌.
  * return 읽어들인 bit값
  */
static uint8_t payload_read_first_bit(payload_t* payload);
/**
  * 남아있는 데이터의 사이즈를 돌려줌.(byte단위)
  */
static int16_t payload_remain_byte(payload_t* payload);
/**
  * 남아있는 데이터의 사이즈를 돌려줌.(bit단위)
  */
static int16_t payload_remain_bit(payload_t* payload);
/**
 * bit단위로 데이터 쓰기
 */
static void payload_write_bit(payload_t* payload, uint8_t bit, int16_t bit_length);
/**
 * bit단위로 데이터 쓰기
 */
static void payload_write_byte_with_bit_length(payload_t* payload, uint8_t* input, int16_t input_byte_length, int16_t bit_length);
/**
 * byte단위로 데이터 쓰기
 */
static void payload_write_byte(payload_t* payload, uint8_t* input, int16_t byte_length);

static void DumpHex(char* buffer, const void* data, int16_t size);

static inline payload_t* payload_create(const uint8_t* pointer, int16_t length, uint8_t initialize) {
  payload_t *payload = (payload_t*)malloc(sizeof(payload_t));
  payload->start = pointer;
  payload->current = (uint8_t*)pointer;
  if (initialize) {
    *(payload->current) = 0x00;
  }
  payload->end = pointer + length;
  payload->size = length;
  payload->remain_bit = 8;
  return payload;
}

static inline void payload_destroy(payload_t* payload) {
  free(payload);
}

static inline uint8_t payload_eof(payload_t* payload) {
  if (payload->current >= payload->end) {
    return 1;
  }
  return 0;
}

static inline uint8_t payload_read_bytes(payload_t* payload, uint8_t* out, int16_t bit_length) {
  int16_t byte_length = (bit_length / 8);
  int16_t remain_bit = (bit_length % 8);
  if(byte_length > 0) {
    if (remain_bit == 0) {
      return payload_read_bytes_ot(payload, out, byte_length);
    }
    int16_t ii;
    for(ii = 0; ii < byte_length; ii ++) {
      *out = payload_read_bit(payload, 8);
      out++; 
    }
    uint8_t data = payload_read_bit(payload, remain_bit);
    *out = (data << (8 - remain_bit));
    return byte_length + 1;
  } else {
    *out = payload_read_bit(payload, bit_length);
    return 1;
  }
}

static inline uint8_t payload_read_bit(payload_t* payload, int16_t bit_length) {
  uint8_t result = 0;
  int16_t ii = 0;
  for (ii = 0; ii < bit_length; ii++) {
    result |= (payload_read_first_bit(payload) << (bit_length - ii - 1));
  }
  return result;
}

static inline uint8_t payload_read_first_bit(payload_t* payload) {
  uint8_t result = 0;
  payload->remain_bit--;
  if (!payload_eof(payload)) {
    result = ((*(payload->current)) >> payload->remain_bit) & 0x01;
  }
  if (payload->remain_bit == 0) {
    payload->current++;
    payload->remain_bit = 8;
  }
  return result;
}

static inline int16_t payload_read_bytes_ot(payload_t* payload, uint8_t* out, int16_t length) {
  int actual_length = length;
  if (payload->end - payload->current < length) {
    actual_length = payload->end - payload->current;
  }

  if (actual_length < 0) {
    actual_length = 0;
  }
  memcpy(out, payload->current, actual_length);
  payload->current += actual_length;
  return actual_length;
}

static inline int16_t payload_remain_byte(payload_t* payload) {
  return (payload->end - payload->current);
}

static inline int16_t payload_remain_bit(payload_t* payload) {
  if (payload->end < payload->current) {
    return 0;
  } else if(payload->end == payload->current) {
    return payload->remain_bit;
  } else {
    int16_t remain_bit = payload->remain_bit;
    remain_bit += ((payload->end - (payload->current + 1)) * 8);
    return remain_bit;
  }
}

static void payload_write_bit(payload_t* payload, uint8_t bit, int16_t bit_length) {
    if (payload_eof(payload)) {
        return;
    }
    int16_t ii;
    for (ii = 0; ii < bit_length; ii++) {
        payload->remain_bit--;
        
        uint8_t data = (bit >> (bit_length - ii - 1)) & 0x01;
        *(payload->current) |= (data << (payload->remain_bit));
        if (payload->remain_bit == 0) {
            payload->current++;
            *(payload->current) = 0x00;
            payload->remain_bit = 8;
        }
    }
}

static inline void payload_write_byte_with_bit_length(payload_t* payload, uint8_t* input, int16_t input_byte_length, int16_t bit_length) {
    if (payload_eof(payload)) {
        return;
    }
    if (payload->remain_bit == 8 && bit_length % 8 == 0) {
        payload_write_byte(payload, input, input_byte_length);
        return;
    }
    
    int16_t ii;
    for (ii = 0; ii < input_byte_length; ii++) {
        if (bit_length < 8) {
            uint8_t temp = (input[ii] >> (8 - bit_length));
            payload_write_bit(payload, temp, bit_length);
        } else {
            payload_write_bit(payload, input[ii], 8);
            bit_length -= 8;
        }
    }
}

static inline void payload_write_byte(payload_t* payload, uint8_t* intput, int16_t byte_length) {
    if (payload->remain_bit == 8) {
        memcpy(payload->current, intput, sizeof(uint8_t) + byte_length);
    } else {
        int data = 0, remain = 0, mask = 0xFF, shift = 0;
        int16_t ii;
        for (ii = 0; ii < byte_length; ii++) {
            mask = 0xFF >> payload->remain_bit;
            shift = (8 - payload->remain_bit);
            
            data = *intput >> shift;
            remain = *intput & mask;
            
            *(payload->current) |= data;
            payload->current++;
            payload->remain_bit = 8;
            
            payload->remain_bit = (8 - shift);
            *(payload->current) = 0x00;
            
            *(payload->current) |= (remain << (payload->remain_bit));
            
            intput++;
        }
    }
}

static inline void DumpHex(char* buffer, const void* data, int16_t size) {
  char ascii[17];
  int16_t i, j;
  int buffer_index = 0;
  ascii[16] = '\0';
  buffer_index += sprintf(buffer + buffer_index, "AMR DumpHex\n");
  for (i = 0; i < size; ++i) {
    buffer_index += sprintf(buffer + buffer_index, "%02X ", ((unsigned char*)data)[i]);
    if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
      ascii[i % 16] = ((unsigned char*)data)[i];
    } else {
      ascii[i % 16] = '.';
    }
    if ((i+1) % 8 == 0 || i+1 == size) {
      buffer_index += sprintf(buffer  + buffer_index, " ");
      if ((i+1) % 16 == 0) {
        buffer_index += sprintf(buffer + buffer_index, "|  %s \n", ascii);
      } else if (i+1 == size) {
        ascii[(i+1) % 16] = '\0';
        if ((i+1) % 16 <= 8) {
          buffer_index += sprintf(buffer + buffer_index, " ");
        }
        for (j = (i+1) % 16; j < 16; ++j) {
          buffer_index += sprintf(buffer + buffer_index, "   ");
        }
        buffer_index += sprintf(buffer + buffer_index,"|  %s \n", ascii);
      }
    }
  }
}
#ifdef __cplusplus
}
#endif
#endif

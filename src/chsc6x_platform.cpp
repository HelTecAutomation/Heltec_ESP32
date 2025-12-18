#include "chsc6x_platform.h"

int chsc6x_platform::semi_touch_i2c_init(void)
{
    //power on
    if((_sda_pin != -1) && (_scl_pin != -1))
    {
        _wire->begin(_sda_pin,_scl_pin,400000);
    }
    else
    {
        _wire->begin();
    }
    // _wire->begin(TOUCH_SDA_PIN,TOUCH_SCL_PIN,400000);
    return 0;
}

int chsc6x_platform::i2cRead(uint8_t i2c_adr, uint16_t reg_adr, uint8_t *rxbuf, uint16_t lenth)
{
    uint8_t buf[2];
    uint8_t reg_adr_len,num = 0;
    if(rxbuf == NULL || lenth == 0)
    {
        return -OS_ERROR;
    }
    if(reg_adr > 0xFF )
    {
        buf[0] = (uint8_t) (reg_adr >> 8);
        buf[1] = (uint8_t) reg_adr ;
        reg_adr_len = 2;
    }
    else
    {
        buf[0] = (uint8_t) reg_adr ;
        reg_adr_len = 1;
    }

    _wire->beginTransmission(i2c_adr); // start transmission to device 
    _wire->write(buf,reg_adr_len); // sends register address to read from
    _wire->endTransmission(0); // end transmission
   
    _wire->requestFrom(i2c_adr,lenth,true);// send data n-bytes read
    while(_wire->available() && (num < lenth))
    {
		  rxbuf[num++] = _wire->read(); // receive DATA
    }
    return num;
}

int chsc6x_platform::i2cSend(uint8_t i2c_adr, uint16_t reg_adr, uint8_t *txbuf, uint16_t lenth)
{
    uint8_t buffer[MAX_IIC_WR_LEN+2] = {0};
    if (NULL==txbuf|| lenth>MAX_IIC_WR_LEN)
    {
        return -OS_ERROR;
    }
    buffer[0] =  reg_adr >> 8;
    buffer[1] =  reg_adr;
    memcpy(&buffer[2], txbuf, lenth);

    _wire->beginTransmission(i2c_adr);
    _wire->write(buffer, lenth+2);
    _wire->endTransmission();		
    return lenth+2;
}

int chsc6x_platform::chsc6x_read_bytes_u8addr(uint8_t id, uint8_t adr, uint8_t *rxbuf, uint16_t lenth)
{
    int ret = i2cRead(id,adr,rxbuf,lenth);
    if(ret == lenth) {
        return OS_OK;
    }else{
        return -OS_ERROR;
    }
}

int chsc6x_platform::chsc6x_write_bytes_u16addr(uint8_t id, uint16_t adr, uint8_t *txbuf, uint16_t lenth)
{
    int ret = i2cSend(id,adr,txbuf,lenth);
    if(ret == lenth+2) {
        return OS_OK;
    }else{
        return -OS_ERROR;
    }
}

int chsc6x_platform::chsc6x_read_bytes_u16addr(uint8_t id, uint16_t adr, uint8_t *rxbuf, uint16_t lenth)
{
    int ret = i2cRead(id,adr,rxbuf,lenth);
    if(ret == lenth) {
        return OS_OK;
    }else{
        return -OS_ERROR;
    }
}

void chsc6x_platform::chsc6x_msleep(int ms)
{
    delay(ms);
}

void chsc6x_platform::chsc6x_tp_reset_delay(chsc6x_reset_e type)
{
    switch (type)
    {
        case HW_CMD_RESET:
            chsc6x_msleep(30);
            break;

        case HW_ACTIVE_RESET:
            chsc6x_msleep(2);
            break;

        default:
            break;
    }

}

int chsc6x_platform::chsc6x_tp_reset(chsc6x_reset_e type)
{
    if (RESET_NONE >= type || RESET_MAX <= type)
    {
        return -OS_ERROR;
    }
    if(_rst_pin != -1) {
        semi_rst_pin_low(_rst_pin);
        chsc6x_msleep(20);
        semi_rst_pin_high(_rst_pin);
        chsc6x_tp_reset_delay(type);
    }
    return OS_OK;
}

bool Touch_state = false;
void blink() {
  Touch_state = !Touch_state;
}
/* GPIO-INT*/
int chsc6x_platform::semi_touch_get_int(void)
{
    if(_int_pin != -1) {
        pinMode(_int_pin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(_int_pin), blink, RISING);
    }
    return _int_pin;
}

/* GPIO-RST*/
int chsc6x_platform::semi_touch_get_rst(void)
{
    if(_rst_pin != -1) {
        pinMode(_rst_pin,OUTPUT);
        digitalWrite(_rst_pin, HIGH);
    }
    return _rst_pin;
}

void chsc6x_platform::semi_rst_pin_low(int pin)
{
    pinMode(pin,OUTPUT);
    digitalWrite(pin, LOW);
}

void chsc6x_platform::semi_rst_pin_high(int pin)
{
    pinMode(pin,OUTPUT);
    digitalWrite(pin, HIGH);
}

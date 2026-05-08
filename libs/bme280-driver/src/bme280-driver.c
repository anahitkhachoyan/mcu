#include "bme280-driver.h"
#include "bme280-regs.h"
#include <stdint.h>
#include <stdio.h>

static bme280_ctx_t bme280_ctx = {0};
static int32_t t_fine;

static uint16_t dig_T1; static int16_t dig_T2, dig_T3;
static uint16_t dig_P1; static int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
static uint8_t  dig_H1, dig_H3; static int16_t dig_H2, dig_H4, dig_H5; static int8_t dig_H6;

void bme280_load_calib() {
    uint8_t buf[24];
    bme280_read_regs(0x88, buf, 24);
    dig_T1 = (buf[1] << 8) | buf[0];
    dig_T2 = (int16_t)((buf[3] << 8) | buf[2]);
    dig_T3 = (int16_t)((buf[5] << 8) | buf[4]);
    dig_P1 = (buf[7] << 8) | buf[6];
    dig_P2 = (int16_t)((buf[9] << 8) | buf[8]);
    dig_P3 = (int16_t)((buf[11] << 8) | buf[10]);
    dig_P4 = (int16_t)((buf[13] << 8) | buf[12]);
    dig_P5 = (int16_t)((buf[15] << 8) | buf[14]);
    dig_P6 = (int16_t)((buf[17] << 8) | buf[16]);
    dig_P7 = (int16_t)((buf[19] << 8) | buf[18]);
    dig_P8 = (int16_t)((buf[21] << 8) | buf[20]);
    dig_P9 = (int16_t)((buf[23] << 8) | buf[22]);

    bme280_read_regs(0xA1, &dig_H1, 1);

    uint8_t h_buf[7];
    bme280_read_regs(0xE1, h_buf, 7);
    dig_H2 = (int16_t)((h_buf[1] << 8) | h_buf[0]);
    dig_H3 = h_buf[2];
    dig_H4 = (int16_t)((h_buf[3] << 4) | (h_buf[4] & 0x0F));
    dig_H5 = (int16_t)((h_buf[5] << 4) | (h_buf[4] >> 4));
    dig_H6 = (int8_t)h_buf[6];
}

void bme280_init(bme280_i2c_read i2c_read, bme280_i2c_write i2c_write) {
    bme280_ctx.i2c_read = i2c_read;
    bme280_ctx.i2c_write = i2c_write;

    uint8_t id_reg_buf[1] = {0};
    bme280_read_regs(BME280_REG_ID, id_reg_buf, sizeof(id_reg_buf));
    if (id_reg_buf[0] != 0x60) {
        printf("BME280 Error: Device ID 0x%02X is incorrect (expected 0x60)\n", id_reg_buf[0]);
        return;
    }

    bme280_load_calib();

    uint8_t ctrl_hum_reg_value = 0;
    ctrl_hum_reg_value |= (0b001 << 0); // osrs_h[2:0] = oversampling 1
    bme280_write_reg(BME280_REG_CTRL_HUM, ctrl_hum_reg_value);

    uint8_t config_reg_value = 0;
    config_reg_value |= (0b0 << 0); // spi3w_en[0:0] = false
    config_reg_value |= (0b000 << 2); // filter[4:2] = Filter off
    config_reg_value |= (0b001 << 5); // t_sb[7:5] = 62.5 ms
    bme280_write_reg(BME280_REG_CONFIG, config_reg_value);

    uint8_t ctrl_meas_value = 0;
    ctrl_meas_value |= (0b11 << 0);  // mode = Normal mode
    ctrl_meas_value |= (0b001 << 2); // osrs_p = x1
    ctrl_meas_value |= (0b001 << 5); // osrs_t = x1
    bme280_write_reg(BME280_REG_CTRL_MEAS, ctrl_meas_value);
}

void bme280_read_regs(uint8_t start_reg_address, uint8_t* buffer, uint8_t length)
{
    uint8_t data[1] = {start_reg_address};
    bme280_ctx.i2c_write(data, sizeof(data));
    bme280_ctx.i2c_read(buffer, length);

}

void bme280_write_reg(uint8_t reg_address, uint8_t value)
{
    uint8_t data[2] = {reg_address, value};
    bme280_ctx.i2c_write(data, sizeof(data));
}

uint16_t bme280_read_temp_raw() {
    uint8_t read[2];
    bme280_read_regs(BME280_REG_TEMP_MSB, read, 2);
    return ((uint16_t)read[0] << 8) | (uint16_t)read[1];
}

uint16_t bme280_read_pres_raw() {
    uint8_t read[2];
    bme280_read_regs(BME280_REG_PRESS_MSB, read, 2);
    return ((uint16_t)read[0] << 8) | (uint16_t)read[1];
}

uint16_t bme280_read_hum_raw() {
    uint8_t buf[2];
    bme280_read_regs(BME280_REG_HUM_MSB, buf, 2);
    return ((uint16_t)buf[0] << 8) | (uint16_t)buf[1];
}

float bme280_get_temperature() {
    uint16_t raw16 = bme280_read_temp_raw();
    
    int32_t adc_T = (int32_t)raw16 << 4;

    int32_t var1, var2;
    
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    
    t_fine = var1 + var2;
    
    float T = (t_fine * 5 + 128) >> 8;
    return T / 100.0f;
}

float bme280_get_pressure() {
    uint16_t raw16 = bme280_read_pres_raw();
    int32_t adc_P = (int32_t)raw16 << 4;

    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

    if (var1 == 0) return 0; 

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    
    return (float)p / 256.0f;
}

float bme280_get_humidity() {
    int32_t adc_H = (int32_t)bme280_read_hum_raw();
    int32_t v_x1_u32r;
    v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v_x1_u32r)) +
                  ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) *
                  (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
                  ((int32_t)2097152)) * ((int32_t)dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    return (float)(v_x1_u32r >> 12) / 1024.0f;
}
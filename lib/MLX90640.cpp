#include "MLX90640_internals.h"
#include "MLX90640.h"

/*!
 *    @brief  Instantiates a new MLX90640 class
 */
MLX90640_DataProcessor::MLX90640_DataProcessor() {
}

/*!
 *    @brief  Sets up the hardware and initializes I2C
 *    @param  i2c_addr
 *            The I2C address to be used.
 *    @param  wire
 *            The Wire object to be used for I2C connections.
 *    @return True if initialization was successful, otherwise false.
 */
void MLX90640_DataProcessor::loadCalibration(uint8_t eeData[1664]) {
    uint16_t* eeData16 = (uint16_t*) eeData;
    for (int i = 0; i < 832; i++) {
        byte_swap_uint16(eeData16[i]);
    }
    // discard error code regarding deviating pixels, since that's apparently a good idea
    MLX90640_ExtractParameters(eeData16, &_params);
}

/*!
 *    @brief  Read 2 pages, calculate temperatures and place into framebuf
 *    @param  framebuf 24*32 floating point memory buffer
 *    @return 0 on success
 */
void MLX90640_DataProcessor::decodeFrame(uint8_t frameData[1672], float framePixels[768]) {
    uint16_t* frameData16 = (uint16_t*) frameData;
    for (int i = 0; i < 836; i++) {
        byte_swap_uint16(frameData16[i]);
    }
    float emissivity = 0.95;
    float tr = 23.15;

    for (uint8_t page = 0; page < 2; page++) {
#ifdef MLX90640_DEBUG
        printf("Page%d = [", page);
        for (int i = 0; i < 834; i++) {
            printf("0x%x, ", mlx90640Frame[i]);
        }
        printf("]\r\n");
#endif
        // For a MLX90640 in the open air the shift is 8 degC.
        tr = MLX90640_GetTa(frameData16, &_params) - OPENAIR_TA_SHIFT;
#ifdef MLX90640_DEBUG
        Serial.print("Tr = ");
        Serial.println(tr, 8);
#endif
        MLX90640_CalculateTo(frameData16, &_params, emissivity, tr, framePixels);
    }
}

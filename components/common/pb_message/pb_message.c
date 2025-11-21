#include "pb_message.h"
#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "result.h"
#include <stdlib.h>

pb_encoding_t pb_message_encode(const void *src_struct,
                                const pb_field_t *fields) {
    pb_encoding_t res = {
        .result = RESULT_ERR_BAD_FORMAT, .data = NULL, .length = 0};
    // Dry RUN with NULL ostream to determine buffer size
    bool status;
    status = pb_get_encoded_size(&res.length, fields, src_struct);

    if (!status) {
        // if dry run faile return res with the error data
        res.length = 0;
        return res;
    }
    res.data = (uint8_t *)malloc(res.length);
    if (res.data == NULL) {
        res.result = RESULT_ERR_NO_MEM;
        res.length = 0;
        return res;
    }

    // Encode message to a real buffer
    pb_ostream_t ostream = pb_ostream_from_buffer(res.data, res.length);
    status = pb_encode(&ostream, fields, src_struct);
    if (!status) {
        free(res.data);
        res.data = NULL;
        res.length = 0;
        res.result = RESULT_FAIL;
        return res;
    }
    res.result = RESULT_OK;
    return res;
}

pb_message_t pb_message_decode(const uint8_t *buffer, size_t size,
                               const pb_field_t *fields, size_t struct_size) {
    pb_message_t res = {.result = RESULT_ERR_BAD_FORMAT, .message = NULL};
    res.message = malloc(struct_size);
    if (res.message == NULL) {
        res.result = RESULT_ERR_NO_MEM;
        return res;
    }
    memset(res.message, 0, struct_size);
    pb_istream_t istream = pb_istream_from_buffer(buffer, size);
    bool status = pb_decode(&istream, fields, res.message);
    if (!status) {
        res.result = RESULT_FAIL;
        free(res.message);
        res.message = NULL;
        return res;
    }
    res.result = RESULT_OK;
    return res;
}

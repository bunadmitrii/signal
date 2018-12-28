#ifndef NET_OP_RESULT_H
#define NET_OP_RESULT_H

enum net_op_result {
    success,
    server_initialization_error = -1,
    connection_establishment_error = -2,
    data_transfer_error = -3,
    connection_closing_error = -4
};

#endif //NET_OP_RESULT_H
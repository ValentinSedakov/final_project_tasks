#include "trn_interface.h"



int main(int argc, char *argv[])
{
    struct transmitter_input inp = {argv[1], argv[2], argv[3]};
    struct transmitter tran1;

    tran1.fil_from_inp = &fill_from_input;
    tran1.sock_estb = &socket_establisher;
    tran1.file_tr = &file_transmission;

    tran1.fil_from_inp(&inp, &tran1);
    tran1.sock_estb(&tran1);
    tran1.file_tr(&tran1);
    
    exit(EXIT_SUCCESS);
}
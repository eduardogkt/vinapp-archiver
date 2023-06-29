#include "libVina++.h"

int main(int argc, char **argv) {

    char *vina_path = NULL;    // caminho para o arquivo vina
    char *target_path = NULL;  // caminho para o arquivo target
    char opcao;

    // le e verifica parametros de entrada
    vinaLeEntrada(argc, argv, &opcao, &vina_path, &target_path);

    switch (opcao) {
        case VINA_OPTION_I: vinaInsere(vina_path, argc, argv, VINA_UPDATE_ALWAYS); break;
        case VINA_OPTION_A: vinaInsere(vina_path, argc, argv, VINA_UPDATE_RECENT); break;
        case VINA_OPTION_M: vinaMove(vina_path, target_path, argc, argv);          break;
        case VINA_OPTION_X: vinaExtrai(vina_path, argc, argv);                     break;
        case VINA_OPTION_R: vinaRemove(vina_path, argc, argv);                     break;
        case VINA_OPTION_C: vinaLista(vina_path, argc, argv);                      break;
        case VINA_OPTION_H: vinaAjuda();                                           break;
        default: return VINA_ERROR_ENTRY;
    }
    return 0;
}

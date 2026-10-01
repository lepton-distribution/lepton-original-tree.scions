/*
 * free : statistiques du tas. L'implementation d'origine (desactivee par #if 0) lisait
 * __iar_dlmallinfo (dlmalloc DLIB d'IAR, Technical Note 28545) ; copie d'origine sous
 * legacy/sys/user/tauon-basic/src/bin/free/free_main.c (D2a). Le tas Lepton n'expose pas
 * de statistiques : la commande ne fait rien, comme avant la migration.
 */

int free_main(int argc,char* argv[])
{
    return 0;
}

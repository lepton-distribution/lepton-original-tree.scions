**Contexte:**
lepton est un système d'exploitation temps réel pour système embarqué à faible empreinte mémoire (fonctionne à partir de 32Ko de RAM).
Il propose une une interface système POSIX (fonctions et appels système) et reprends la philosophie UNIX: tout est fichier, tout est flux de données.
l'achitecture en résumé:
kernel: dans le répertoire kernel sont impémentés tous les service proposé par le noyau de lepton.
dev: dans ce répertoire se trouve l'ensemble des pilotes de périphérique qu'il soit matériel ou logiciel.
lib: dans ce répertoire se trouve l'ensemble des librairies qui utilisent les fonctions et appels système POSIX.
sbin: contient les pseudo binaires des utilitaires système
bin: contient les pseudo binaires spécifiques à l'application utilisateur

**Ce qu'il faut faire:**
1.1) porter le noyau lepton statique, ce sera une librairie spécifique:
compiler le noyau pour qu'il fonctionne sur le système hôte (linux) directement en statique, pas de scheduler, inutile de compiler les fonctions dépendant du scheduler:
- kernel/core: base du noyau 
- kernel/dev: uniquement les pilotes de pérphérique logiciel kernel/dev/dev_cpufs, kernel/dev/dev_head, kernel/dev/dev_null, kernel/dev/dev_null, kernel/dev/dev_proc, kernel/dev/dev_tty
- kernel/fs: en premier le système de fichier virtuel kernel/fs/vfs, ensuite dans cet ordre: kernel/fs/rootfs qui est le premier système de fichier charger par le noyau (il est en ram ne nécessite pas de pilote de périphérique), kernel/fs/ufs, sustème de fichier de base utilisé ensuite par mklepton.   
1.2) portage mklepton:
mklepton permet de générer les fichier de configuration du noyau lepton, définit les pseudo-binaires utilisés et les fichiers provenant du système hôte pour les intégrer dans un système de de fichier (un système de fichier de lepton nommé ufs).
Ce système de fichier est placé sur un périphérique dev_cpufs (kernel/dev/dev_cpufs/). il sera monté au démarrage du noyau.
ce disque contient dans sont arborescence les fichiers des pseudo-binaires, et fichiers qui seront utilisés.

mklepton utilise le noyau lepton statique, sans scheduler et appels système.

2.1) porter le noyau lepton dynamique
compiler le noyau lepton pour qu'il fonctionne sur une cible micro-controleur émuler avec QEMU. scheduler activé.
- kernel/core/core-segger: wrapper primitive noyau temps réel segger 
- kernel/core/kal: kernel abstraction layer spécifique au noyau temps réel et à la cible matériel utilisé 
- kernel/core: base du noyau avec tous les appels systèmes
- kernel/dev: avec les pilote de périphérique spécifique à la cible QEMU choisi, pour le premier test valider avec la liaison série.
- kernel/fs: dans un premier temps kernel/fs/vfs, kernel/fs/rootfs, kernel/fs/ufs.
- kernel/sbin: pseudo binaires de base du système (lsh, ps, ls, uname).
- kernel/bin: pseudo binaires de tests unitaires (à définir).

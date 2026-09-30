# Étape 7 — Backend KAL FreeRTOS

## Contexte

La migration est faite sur le backend embOS. Cette étape ajoute un backend KAL **FreeRTOS**
sans toucher au compilateur, au noyau ni aux cartes : seul l'axe micro-noyau change
(`LEPTON_KAL_BACKEND=freertos`). Un répertoire `core-freertos` existe déjà dans l'arbre
(relevé à l'étape 0) : l'étape 1 en a établi l'état ; on part de lui, pas de zéro. Critère de
succès : même comportement qu'embOS, test par test, sur les mêmes machines.

## Prérequis

- Étape 6 close (ou au minimum l'étape 5 : socle QEMU et F439 verts sous embOS).
- Version FreeRTOS épinglée (LTS), vendored ; `cartographie-kal.md` (état de `core-freertos`).

## Tâches

### 1. Analyse d'écart

- Pour chaque macro du KAL (`__is_thread_self`, `__bckup_context`, `__rstr_context`,
  `__bckup_stack`, `__swap_signal_handler`, `__stop_sched`…), l'équivalent FreeRTOS : API publique
  quand elle suffit (`xTaskGetCurrentTaskHandle`, pointeurs de stockage local de tâche pour lier
  pthread et tâche), accès interne sinon — chaque accès interne documenté dans
  `doc/migration/kal-freertos-ecarts.md`.
- Comparer avec ce que fait déjà `core-freertos` ; ne réécrire que l'écart.
- Points à trancher : format de contexte FreeRTOS et opérations vfork/exec/signaux ; cohabitation
  de l'appel système Lepton (SVC) avec l'usage de SVC/PendSV par le port FreeRTOS (numéros distincts,
  `configMAX_SYSCALL_INTERRUPT_PRIORITY`) ; branchement du tick.

### 2. Implémentation

- `kal/backend/freertos/` dans la structure de l'étape 4 ; `arch/` réutilisé tel quel ;
  `FreeRTOSConfig.h` par carte ; presets `<carte>-freertos`.
- Aucune modification hors `kal/backend/freertos/` et configuration.

### 3. Validation comparée

- Banc KAL colonne FreeRTOS (T0-T11) sur `mps2-an386`, puis paliers de l'étape 5 sur la F439.
- Comparer test par test avec la colonne embOS : signaux, préemption (convention de priorités
  inversée), timers, empreinte mémoire.
- CI : matrice micro-noyau × machine.

## Critères de validation

- [ ] `kal-freertos-ecarts.md` complet.
- [ ] Banc KAL identique sur les deux backends ; écarts documentés et acceptés.
- [ ] F439 validée en FreeRTOS (paliers 1 à 8 de l'étape 5).
- [ ] Zéro modification hors `kal/backend/freertos/`.

## Pièges connus

- Convention de priorités inversée entre embOS et FreeRTOS : à traduire dans le KAL.
- `configASSERT` et `configCHECK_FOR_STACK_OVERFLOW=2` actifs pendant toute la validation.
- Le port FreeRTOS suppose contrôler PendSV, SysTick et SVC : revoir chaque exception partagée.

## À la fin de l'étape

`MIGRATION-STATUS.md` : matrice micro-noyau × cible ; devenir du backend embOS (maintenu ou
déprécié) ; `handoff/etape-7.md`.

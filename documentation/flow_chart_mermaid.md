flowchart LR
    %% === STYLING ===
    classDef startEnd fill:#4CAF50,stroke:#2E7D32,color:#fff,stroke-width:2px
    classDef process fill:#42A5F5,stroke:#1976D2,color:#fff,stroke-width:2px
    classDef decision fill:#FFA726,stroke:#EF6C00,color:#fff,stroke-width:2px
    classDef inputOutput fill:#AB47BC,stroke:#7B1FA2,color:#fff,stroke-width:2px

    %% === START ===
    START([🎮 START]):::startEnd

    %% === MENU MAIN (NEW) ===
    START --> MENU_MAIN["📍 MENU_MAIN<br/>Tampilkan Main Menu<br/>Start / History / Quit"]:::process
    
    MENU_MAIN --> MAIN_INPUT[/"⌨️ Input: [1] Start , [2] History , [Q] Quit , [S] SFX"/]:::inputOutput
    
    MAIN_INPUT --> MAIN_DEC{"Tombol<br/>yang ditekan?"}:::decision
    
    MAIN_DEC -->|"Q"| EXIT([🔚 EXIT / END]):::startEnd
    MAIN_DEC -->|"S"| TOGGLE_SFX["Toggle SFX On/Off"]:::process
    TOGGLE_SFX --> MENU_MAIN
    MAIN_DEC -->|"1"| MENU_LANG
    MAIN_DEC -->|"2"| MENU_HISTORY

    %% === MENU HISTORY (NEW) ===
    MENU_HISTORY["📍 MENU_HISTORY<br/>Tampilkan History<br/>dengan Pagination"]:::process
    
    MENU_HISTORY --> HIST_INPUT[/"⌨️ Input: [N] Next , [P] Prev , [C] Clear , [B] Back"/]:::inputOutput
    
    HIST_INPUT --> HIST_DEC{"Tombol<br/>yang ditekan?"}:::decision
    
    HIST_DEC -->|"B / ESC"| MENU_MAIN
    HIST_DEC -->|"N"| NEXT_PAGE["Next Page<br/>(jika ada)"]:::process
    HIST_DEC -->|"P"| PREV_PAGE["Prev Page<br/>(jika ada)"]:::process
    HIST_DEC -->|"C"| CLEAR_CONFIRM{"Konfirmasi<br/>Clear?"}:::decision
    
    NEXT_PAGE --> MENU_HISTORY
    PREV_PAGE --> MENU_HISTORY
    CLEAR_CONFIRM -->|"Ya"| CLEAR_HIST["Hapus Semua<br/>History"]:::process
    CLEAR_CONFIRM -->|"Tidak"| MENU_HISTORY
    CLEAR_HIST --> MENU_HISTORY

    %% === MENU LANGUAGE ===
    MENU_LANG["📍 MENU_LANGUAGE<br/>Tampilkan Menu Pilihan Bahasa"]:::process
    
    MENU_LANG --> LANG_INPUT[/"⌨️ Input: [1] ID , [2] EN , [B] Back , [S] SFX"/]:::inputOutput
    
    LANG_INPUT --> LANG_DEC{"Tombol<br/>yang ditekan?"}:::decision
    
    LANG_DEC -->|"B"| MENU_MAIN
    LANG_DEC -->|"1 atau 2"| SET_LANG["Set Bahasa<br/>(ID atau EN)"]:::process

    %% === MENU DURATION ===
    SET_LANG --> MENU_DUR["📍 MENU_DURATION<br/>Tampilkan Menu Durasi"]:::process
    
    MENU_DUR --> DUR_INPUT[/"⌨️ Input: [1] 15s , [2] 30s , [3] 60s , [4] Custom , [5] Tanpa Batas , [B] Back"/]:::inputOutput
    
    DUR_INPUT --> DUR_DEC{"Tombol<br/>yang ditekan?"}:::decision
    
    DUR_DEC -->|"B"| MENU_LANG
    DUR_DEC -->|"4"| CUSTOM_DUR[/"⌨️ Input Custom Duration (detik)"/]:::inputOutput
    CUSTOM_DUR --> SET_DUR["Set Durasi"]:::process
    DUR_DEC -->|"1, 2, 3, atau 5"| SET_DUR

    %% === MENU MODE ===
    SET_DUR --> MENU_MODE["📍 MENU_MODE<br/>Tampilkan Menu Mode"]:::process
    
    MENU_MODE --> MODE_INPUT[/"⌨️ Input: [1] Manual , [2] Campaign , [B] Back"/]:::inputOutput
    
    MODE_INPUT --> MODE_DEC{"Tombol<br/>yang ditekan?"}:::decision
    
    MODE_DEC -->|"B"| MENU_DUR
    MODE_DEC -->|"1"| MANUAL_MODE["Mode = Manual"]:::process
    MODE_DEC -->|"2"| CAMPAIGN_MODE["Mode = Campaign"]:::process

    %% === MANUAL MODE PATH ===
    MANUAL_MODE --> TARGET_INPUT[/"⌨️ Input Target WPM<br/>[ESC] untuk kembali"/]:::inputOutput
    
    TARGET_INPUT --> TARGET_DEC{"ESC<br/>ditekan?"}:::decision
    TARGET_DEC -->|"Ya"| MENU_MODE
    TARGET_DEC -->|"Tidak"| SET_TARGET["Set Target WPM"]:::process
    SET_TARGET --> PLAYING

    %% === CAMPAIGN MODE PATH ===
    CAMPAIGN_MODE --> MENU_DIFF["📍 MENU_DIFFICULTY<br/>Tampilkan Menu Difficulty<br/>Easy/Medium/Hard/Programmer"]:::process
    
    MENU_DIFF --> DIFF_INPUT[/"⌨️ Input: [1] Easy , [2] Medium , [3] Hard , [4] Programmer , [B] Back , [C] Credits"/]:::inputOutput
    
    DIFF_INPUT --> DIFF_DEC{"Tombol<br/>yang ditekan?"}:::decision
    
    DIFF_DEC -->|"B / ESC"| MENU_MODE
    DIFF_DEC -->|"C"| CREDITS
    DIFF_DEC -->|"1-4"| CHECK_UNLOCK{"Level<br/>Unlocked?"}:::decision
    
    CHECK_UNLOCK -->|"Tidak"| MENU_DIFF
    CHECK_UNLOCK -->|"Ya"| SET_DIFF["Set Difficulty Level"]:::process
    SET_DIFF --> PLAYING

    %% === PLAYING STATE ===
    PLAYING["📍 PLAYING<br/>🎯 Game Loop Aktif<br/>Tampilkan Kata + Timer"]:::process
    
    PLAYING --> PLAY_INPUT[/"⌨️ Input Karakter<br/>[ESC] Exit , [TAB] Restart"/]:::inputOutput
    
    PLAY_INPUT --> PLAY_DEC{"Tombol<br/>yang ditekan?"}:::decision
    
    PLAY_DEC -->|"ESC"| CHECK_MODE_BACK{"Mode<br/>Campaign?"}:::decision
    CHECK_MODE_BACK -->|"Ya"| MENU_DIFF
    CHECK_MODE_BACK -->|"Tidak"| MENU_MODE
    
    PLAY_DEC -->|"TAB"| RESTART["Reset Session"]:::process
    RESTART --> PLAYING
    
    PLAY_DEC -->|"Karakter"| PROCESS_CHAR["Proses Karakter<br/>Update Statistik"]:::process
    PROCESS_CHAR --> CHK_END{"Waktu Habis<br/>atau Selesai?"}:::decision
    
    CHK_END -->|"Tidak"| PLAYING
    CHK_END -->|"Ya"| SAVE_HISTORY["Simpan ke History"]:::process
    SAVE_HISTORY --> RESULTS

    %% === RESULTS STATE ===
    RESULTS["📍 RESULTS<br/>Tampilkan Statistik<br/>WPM, Accuracy, Time, Errors"]:::process
    
    RESULTS --> HITUNG["Hitung Statistik Final"]:::process
    
    HITUNG --> CHK_CAMPAIGN{"Mode<br/>Campaign?"}:::decision
    
    CHK_CAMPAIGN -->|"Tidak (Manual)"| RESULT_OUTPUT[/"📊 Output: Hasil vs Target WPM"/]:::inputOutput
    
    CHK_CAMPAIGN -->|"Ya"| CHK_PASS{"Target<br/>Tercapai?"}:::decision
    
    CHK_PASS -->|"Tidak"| FAIL_MSG[/"📊 Output: Target Missed<br/>Tampilkan Requirement"/]:::inputOutput
    
    CHK_PASS -->|"Ya"| UNLOCK["✅ Unlock Level Berikutnya<br/>Simpan Progress"]:::process
    UNLOCK --> PASS_MSG[/"📊 Output: Level Passed!<br/>Next Level Unlocked"/]:::inputOutput
    
    %% === RICK ROLL EASTER EGG ===
    UNLOCK --> CHK_HARD{"Hard Mode<br/>Baru Selesai?"}:::decision
    CHK_HARD -->|"Tidak"| RESULT_WAIT
    CHK_HARD -->|"Ya (Pertama Kali)"| RICK["🎵 Easter Egg:<br/>Rick Roll Animation!"]:::process
    RICK --> CREDITS

    RESULT_OUTPUT --> RESULT_WAIT
    FAIL_MSG --> RESULT_WAIT
    PASS_MSG --> RESULT_WAIT
    
    RESULT_WAIT[/"⌨️ Input: [C] Credits , Any Key"/]:::inputOutput
    
    RESULT_WAIT --> RES_KEY{"Tombol C<br/>ditekan?"}:::decision
    RES_KEY -->|"Ya"| CREDITS
    RES_KEY -->|"Tidak (Any Key)"| CHK_MODE_RES{"Mode<br/>Campaign?"}:::decision
    CHK_MODE_RES -->|"Ya"| MENU_DIFF
    CHK_MODE_RES -->|"Tidak"| MENU_MODE

    %% === CREDITS STATE ===
    CREDITS["📍 CREDITS<br/>Tampilkan Credits"]:::process
    
    CREDITS --> CRED_INPUT[/"⌨️ Input: Any Key untuk kembali"/]:::inputOutput
    
    CRED_INPUT --> CRED_BACK["Kembali ke State Sebelumnya"]:::process
    CRED_BACK --> MENU_DIFF

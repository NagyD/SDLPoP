add_subdirectory(src)

FILE(
    GLOB_RECURSE
    data_dos_file
    data/*
)

add_custom_target(Data_DOS
    SOURCES
    ${data_dos_file}
)

FILE(
    GLOB_RECURSE
    doc_file
    doc/*
)

add_custom_target(Doc
    SOURCES
    ${doc_file}
)

add_custom_target(Root_Data
    SOURCES
    COPYING
    ReadMe.amigaos4
    README.md
    SDLPoP.ini
    mods/mods.txt
    replays/replays.txt
)

$display_include_tree=$args[0]

if($display_include_tree -eq 'show' -or $display_include_tree -eq 'tree') {
    $pattern = '"^\.+ (engine|src|include)"'
    $command = 'g++ -H -c main.cpp networking/*.cpp engine/*.cpp engine/ECS/*.cpp engine/ECS/Colliders/*.cpp -IC:/msys64/mingw64/include/SDL2 -IC:/msys64/mingw64/include/asio -IN:/game_dev/game_000/networking -LC:/msys64/mingw64/lib  -lmingw32 -lSDL2main -lSDL2 -lws2_32 -lwsock32 -lSDL2_mixer -lSDL2_image -lSDL2_ttf'
    
    Invoke-Expression "$command 2>&1 | Select-String -Pattern $pattern"
    Remove-Item "*.o"
}

Start-Process make -NoNewWindow -Wait
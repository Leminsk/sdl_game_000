$display_include_tree=$args[0]

if($display_include_tree -eq 'show' -or $display_include_tree -eq 'tree') {
    $pattern = '"^\.+ (engine|src|include)"'
    $command = 'g++ -H -c main.cpp engine/*.cpp engine/ECS/*.cpp -IC:/msys64/mingw64/include/SDL2  -LC:/msys64/mingw64/lib  -lmingw32 -lSDL2main -lSDL2   -lSDL2_image'
    
    Invoke-Expression "$command 2>&1 | Select-String -Pattern $pattern"
    Remove-Item "*.o"
}

Start-Process make -NoNewWindow -Wait
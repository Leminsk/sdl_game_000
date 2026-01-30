$build_args=$args[0]

if($build_args -eq 'clean') {
    Remove-Item "*.o"
    Remove-Item "engine\*.o"
    Remove-Item "engine\ECS\*.o"
    Remove-Item "engine\ECS\Colliders\*.o"

} elseif($build_args -eq 'show' -or $build_args -eq 'tree') {

    $pattern = "^\.+ (engine|src|include)"
    $makeOutput = & "C:\msys64\usr\bin\bash.exe" -c "make tree=y -n"
    $gppCommandLine = $makeOutput | Where-Object { $_ -match "g\+\+" }
    if (-not $gppCommandLine) {
        Write-Error "Could not find the g++ command in Makefile output."
        exit 1
    }
    Write-Host "`n[Running]: $gppCommandLine"
    Invoke-Expression "$gppCommandLine 2>&1 | Select-String -Pattern '$pattern'"
    Remove-Item "*.o"

} elseif($build_args -eq 'debug') {
    Start-Process -FilePath "make" -ArgumentList "debug=y" -NoNewWindow -Wait
} else {
    Start-Process -FilePath "make" -NoNewWindow -Wait
}

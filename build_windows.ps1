$display_include_tree=$args[0]

if($display_include_tree -eq 'show' -or $display_include_tree -eq 'tree') {

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

} elseif($display_include_tree -eq 'debug') {
    Start-Process -FilePath "make" -ArgumentList "debug=y" -NoNewWindow -Wait
} else {
    Start-Process -FilePath "make" -NoNewWindow -Wait
}

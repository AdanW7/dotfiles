$GitSubModuleDeInit = ('Recursively Deinitialize SubModules', 'git submodule deinit --all --force', $false, @(0))
$GitSubModuleInit = ('Recursively Initialize SubModules', 'git submodule update --init --recursive', $false, @(0))


$CommandArr = $GitSubModuleDeInit, $GitSubModuleInit

For ($i = 0; $i -lt $CommandArr.Length; $i++)
{

    Write-Host '================================'
    Write-Host        $CommandArr[$i][0]
    Write-Host '================================'
    Invoke-Expression $CommandArr[$i][1]

    if ($CommandArr[$i][2] -or ($CommandArr[$i][3] -ccontains $LASTEXITCODE))
    {
        Write-host ("{0}: PASSED" -f $CommandArr[$i][0]) -ForegroundColor Green
    } else
    {
        Write-host ("{0}: FAILED" -f $CommandArr[$i][0]) -ForegroundColor Red
        break
    }
}

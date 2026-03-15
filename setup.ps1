$GitSubModuleDeInit = @(
    'Recursively Deinitialize SubModules'
    @('git', 'submodule', 'deinit', '--all', '--force')
    $false
    @(0)
)
$GitSubModuleInit = @(
    'Recursively Initialize SubModules'
    @('git', 'submodule', 'update', '--init', '--recursive')
    $false
    @(0)
)


$CommandArr = $GitSubModuleDeInit, $GitSubModuleInit

for ($i = 0; $i -lt $CommandArr.Length; $i++) {
    Write-Output '================================'
    Write-Output $CommandArr[$i][0]
    Write-Output '================================'

    $cmd = $CommandArr[$i][1]
    & $cmd[0] @($cmd[1..($cmd.Length - 1)])

    if ($CommandArr[$i][2] -or ($CommandArr[$i][3] -ccontains $LASTEXITCODE)) {
        Write-Output ("{0}: PASSED" -f $CommandArr[$i][0])
    }
    else {
        Write-Output ("{0}: FAILED" -f $CommandArr[$i][0])
        break
    }
}

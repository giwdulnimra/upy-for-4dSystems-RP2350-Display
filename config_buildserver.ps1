# ---------------- CONFIG -----------------
$RemoteUser     = "armin"
$RemoteHost     =
    "10.19.28.28"
    #"code.protronic.local"
    #"can-mon"
scp .\config_build.sh "${RemoteUser}@${RemoteHost}:~/config/config_build.sh"
ssh $RemoteUser@$RemoteHost "bash ~/config_build.sh"
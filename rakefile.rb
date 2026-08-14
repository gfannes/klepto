task :upload do
    sh 'mpremote exec "from mpos import AppManager; AppManager.restart_launcher()"'
    sleep 0.5
    sh 'mpremote fs cp mp/main.py :/apps/com.fannes.klepto/main.py'
    sleep 0.5
    sh "mpremote exec \"from mpos import AppManager; AppManager.start_app('com.fannes.klepto')\""
end

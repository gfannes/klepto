def mp_mkdir(path)
    sh "mpremote fs ls #{path}", verbose: false do |ok, _res|
        return if ok
    end
    sh "mpremote fs mkdir #{path}"
end

task :install do
    sh 'mpremote exec "from mpos import AppManager; AppManager.restart_launcher()"'
    sleep 0.2
    mp_mkdir ':/apps'
    mp_mkdir ':/apps/com.fannes.klepto'
    sh 'mpremote fs cp mp/main.py :/apps/com.fannes.klepto/main.py'
    sh 'mpremote fs cp mp/MANIFEST.JSON :/apps/com.fannes.klepto/MANIFEST.JSON'
    sh 'mpremote fs cp mp/icon_64x64.png :/apps/com.fannes.klepto/icon_64x64.png'
end
task :upload do
    sh 'mpremote exec "from mpos import AppManager; AppManager.restart_launcher()"'
    sleep 0.2
    sh 'mpremote fs cp mp/main.py :/apps/com.fannes.klepto/main.py'
    sh "mpremote exec \"from mpos import AppManager; AppManager.start_app('com.fannes.klepto')\""
end
task :run do
    sh 'mpremote exec "from mpos import AppManager; AppManager.restart_launcher()"'
    sleep 0.2
    sh "mpremote exec \"from mpos import AppManager; AppManager.start_app('com.fannes.klepto')\""
end

import os
import subprocess

STEAM_EXE = 'C:\Program Files (x86)\Steam\steam.exe'
STEAM_USERNAME = 'wasabimilkshake'
STEAM_USERID = 76561197975668032
DS1R_EXE = 'Q:\\SteamLibrary\\steamapps\\common\\DARK SOULS REMASTERED\\DarkSoulsRemastered.exe'
DS1R_DIR = 'Q:\\SteamLibrary\\steamapps\\common\\DARK SOULS REMASTERED'
DS1R_APPID = 570940

if __name__ == '__main__':
    env = os.environ.copy()
    env['SteamAppId'] = '570940'

    '''
    env['SteamPath'] = 'C:\\Program Files (x86)\\Steam'

    for k in ['SteamClientLaunch', 'SteamEnv', 'SteamNoOverlayUI']:
        env[k] = '1'

    for k in ['SteamAppId', 'SteamOverlayGameId']:
        env[k] = '570940'

    for k in ['SteamAppUser', 'SteamUser']:
        env[k] = 'wasabimilkshake'

    env['STEAMID'] = '76561197975668032'

    env['SteamAppId'] = '570940'
    env['SteamAppUser'] = 'wasabimilkshake'
    env['STEAMID'] = '76561197975668032'
    '''
    print(subprocess.call([DS1R_EXE], env=env, cwd=DS1R_DIR))

'''
set APP_ID=570940
	SteamAppId
	SteamOverlayGameId

set USER_NAME=wasabimilkshake
	SteamAppUser
	SteamUser

set USER_ID=76561197975668032
	STEAMID


SteamClientLaunch 1
SteamEnv 1
SteamNoOverlayUI 1
SteamPath C:\Program Files (x86)\Steam
'''

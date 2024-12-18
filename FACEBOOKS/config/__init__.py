from .app_config import AppConfig

if not AppConfig.is_initialized:
    AppConfig.init()
    AppConfig.is_initialized = True

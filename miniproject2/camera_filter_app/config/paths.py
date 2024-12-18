from .app_config import AppConfig


class AssetPaths:
    FACE_SUNGLASSES_PATH: str = f"{AppConfig.resource_path}/sunglasses.png"
    FACE_RABBIT_EARS_PATH: str = f"{AppConfig.resource_path}/rabbit_ears.png"
    FACE_MOUTH_MASK_PATH: str = f"{AppConfig.resource_path}/mouth_mask.png"

    BACKGROUND_PHOTO_ZONE_PATH: str = f"{AppConfig.resource_path}/back.jpg"
    BACKGROUND_SPACE_PATH: str = f"{AppConfig.resource_path}/space.jpg"
    BACKGROUND_OCEAN_PATH: str = f"{AppConfig.resource_path}/ocean.jpg"

from pathlib import Path


class AppConfig:
    """It will be initialized when imported refer to __init__.py"""

    is_initialized: bool = False

    PROJECT_ROOT_NAME: str = "camera_filter_app"
    project_root: Path = Path()
    resource_path: Path = Path()
    output_path: Path = Path()

    @staticmethod
    def init():
        current_file_path = Path(__file__).resolve()
        project_root: Path = current_file_path
        while project_root.name != AppConfig.PROJECT_ROOT_NAME:
            project_root = project_root.parent

        AppConfig.project_root = project_root
        AppConfig.resource_path = project_root / "rsrc"
        AppConfig.output_path = project_root / "output" / "images"
        AppConfig.output_path.mkdir(parents=True, exist_ok=True)

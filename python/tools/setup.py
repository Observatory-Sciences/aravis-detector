from setuptools import setup

setup(
    name = "aravis_detector_cli",
    version = '0.0.1',
    packages=['python_cli'],
    entry_points = {
        'console_scripts':[
            'arvcli = python_cli.__main__:main'
        ]
    }
)
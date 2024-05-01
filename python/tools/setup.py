from setuptools import setup

setup(
    name="aravis_detector_cli",
    version='0.0.1',
    packages=['arvcli'],
    entry_points={
        'console_scripts':
        [
            'arvcli = arvcli.__main__:main'
        ]
    }
)

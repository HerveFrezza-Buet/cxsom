
import setuptools

setuptools.setup(
    name = "pycxsom",
    version = "1.0",
    scripts = ['bin/pycxsom-plot-as-lines.py',
               'bin/cxsom-monitor-situation-report.py'],
    packages = setuptools.find_packages(),
)

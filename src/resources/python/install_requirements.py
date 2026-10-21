"""Bootstrap authenticated pip before downloading plugin dependencies."""
import os
import subprocess
import sys

# Independently published by the Python security team for the eMudhra incident:
# https://discuss.python.org/t/109576
PIP_WHEEL = (
    "https://files.pythonhosted.org/packages/3f/06/"
    "3e4420302363fbb71be3c7f35ce042f54bfca330e3e66b3a2e3c4cca579f/"
    "pip-26.2.2-py3-none-any.whl"
    "#sha256=fd7f4dfa8190e014013b9f08f3e973f4e4c26ffd179a204895533a7254813a70"
)


def install(requirements):
    if sys.version_info < (3, 10):
        raise RuntimeError("Plugin dependency installation requires Python 3.10 or newer.")
    pip = [sys.executable, "-m", "pip", "--disable-pip-version-check"]
    # Old pip authenticates this wheel using its fixed SHA256 before executing it.
    subprocess.check_call(pip + ["install", "--no-deps", "--upgrade", PIP_WHEEL])
    # Native roots can reintroduce the compromised trust even after pip is patched.
    env = dict(os.environ, PIP_USE_DEPRECATED="legacy-certs")
    # Explicit requirements prevent a plugin from downgrading either trust bundle.
    subprocess.check_call(pip + [
        "install", "--use-deprecated=legacy-certs",
        "pip>=26.2.2", "certifi>=2026.10.19", "-r", requirements,
    ], env=env)


if __name__ == "__main__":
    install(sys.argv[1])

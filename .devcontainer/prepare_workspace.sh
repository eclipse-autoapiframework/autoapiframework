#!/usr/bin/env sh
set -eu

# Only needed behind a corporate proxy with a custom CA: uncomment the next line and
# set the URL of your CA bundle. Do not check in this change.
#CA_BUNDLE_URL=https://your-server/cabundle.crt

workspace="$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd)"
venv="$workspace/.venv-vss-tools"

as_root() {
	if [ "$(id -u)" -eq 0 ]; then
		"$@"
	else
		sudo "$@"
	fi
}

if [ -n "${CA_BUNDLE_URL:-}" ]; then
	bundle="$(mktemp)"
	trap 'rm -f "$bundle"' EXIT
	echo "[INFO] Downloading CA bundle from ${CA_BUNDLE_URL} and updating system trust store..."
	wget "${CA_BUNDLE_URL}" -O "$bundle" --no-check-certificate
	as_root awk 'split_after == 1 {n++;split_after=0} /-----END CERTIFICATE-----/ {split_after=1} {print > "/usr/local/share/ca-certificates/cert" n ".crt"}' < "$bundle"
	as_root update-ca-certificates
fi

as_root apt-get update
as_root env DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
	cmake ninja-build g++ libwxgtk3.2-dev python3-venv
python3 -m venv "$venv"
"$venv/bin/pip" install vss-tools==6.1.0
"$venv/bin/vspec" --help > /dev/null

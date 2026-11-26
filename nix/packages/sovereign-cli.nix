{ lib, stdenv, cmake, ninja, libargon2, src }:

stdenv.mkDerivation rec {
  pname = "sovereign-cli";
  version = "0.2.0-os2";

  inherit src;

  nativeBuildInputs = [ cmake ninja ];
  buildInputs = [ libargon2 ];

  cmakeFlags = [
    "-DSOVEREIGN_BUILD_TESTS=OFF"
    "-DCMAKE_BUILD_TYPE=Release"
  ];

  installPhase = ''
    runHook preInstall
    mkdir -p $out/bin
    CLI=$(find . -name sovereign-cli -type f -executable | head -1)
    install -m755 "$CLI" $out/bin/sovereign-cli
    install -m755 "$src/scripts/sovereign-node-install.sh" $out/bin/sovereign-node-install
    install -m755 "$src/scripts/sovereign-install-pack.sh" $out/bin/sovereign-install-pack
    mkdir -p $out/share/sovereign-hub
    cp -f "$src/tools/mission_control/"*.py "$src/tools/mission_control/"*.html $out/share/sovereign-hub/
    runHook postInstall
  '';

  meta = with lib; {
    description = "Sovereign OS command-line interface";
    license = licenses.mit;
    platforms = platforms.linux;
    mainProgram = "sovereign-cli";
  };
}

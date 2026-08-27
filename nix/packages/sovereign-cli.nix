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
    runHook postInstall
  '';

  meta = with lib; {
    description = "Sovereign OS command-line interface";
    license = licenses.mit;
    platforms = platforms.linux;
    mainProgram = "sovereign-cli";
  };
}

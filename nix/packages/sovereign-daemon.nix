{ lib
, stdenv
, cmake
, ninja
, libargon2
}:

stdenv.mkDerivation rec {
  pname = "sovereign-daemon";
  version = "0.2.0-os2";

  src = ../../.;

  nativeBuildInputs = [ cmake ninja ];
  buildInputs = [ libargon2 ];

  cmakeFlags = [
    "-DSOVEREIGN_BUILD_TESTS=OFF"
    "-DCMAKE_BUILD_TYPE=Release"
  ];

  installPhase = ''
    runHook preInstall
    mkdir -p $out/bin
    if [ -x apps/daemon/sovereign-daemon ]; then
      install -m755 apps/daemon/sovereign-daemon $out/bin/sovereign-daemon
    else
      DAEMON=$(find . -name sovereign-daemon -type f -executable | head -1)
      install -m755 "$DAEMON" $out/bin/sovereign-daemon
    fi
    runHook postInstall
  '';

  meta = with lib; {
    description = "Sovereign OS continuous agent daemon";
    license = licenses.mit;
    platforms = platforms.linux;
    mainProgram = "sovereign-daemon";
  };
}

{ lib
, stdenv
, cmake
, ninja
, libargon2
, python3
}:

stdenv.mkDerivation rec {
  pname = "sovereign-cli";
  version = "0.2.0-os2";

  # Repo root is two levels up from nix/packages
  src = ../../.;

  nativeBuildInputs = [ cmake ninja ];
  buildInputs = [ libargon2 ];

  # Avoid pulling tests into the runtime image by default
  cmakeFlags = [
    "-DSOVEREIGN_BUILD_TESTS=OFF"
    "-DCMAKE_BUILD_TYPE=Release"
  ];

  # Source tree has nested CMake; build from root
  dontUseCmakeConfigure = false;

  preConfigure = ''
    # Ensure argon2 is found
    export CMAKE_PREFIX_PATH="${libargon2}:$CMAKE_PREFIX_PATH"
  '';

  installPhase = ''
    runHook preInstall
    mkdir -p $out/bin
    if [ -x apps/cli/sovereign-cli ]; then
      install -m755 apps/cli/sovereign-cli $out/bin/sovereign-cli
    elif [ -x bin/sovereign-cli ]; then
      install -m755 bin/sovereign-cli $out/bin/sovereign-cli
    else
      # Fallback: find binary
      CLI=$(find . -name sovereign-cli -type f -executable | head -1)
      install -m755 "$CLI" $out/bin/sovereign-cli
    fi
    runHook postInstall
  '';

  meta = with lib; {
    description = "Sovereign OS command-line interface";
    license = licenses.mit;
    platforms = platforms.linux;
    mainProgram = "sovereign-cli";
  };
}

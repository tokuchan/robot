FROM docker.io/nixos/nix:latest

WORKDIR /workspace

RUN mkdir -p /etc/nix && echo "experimental-features = nix-command flakes" >> /etc/nix/nix.conf

COPY flake.nix ./

RUN if [ ! -f flake.lock ]; then nix flake update; fi

COPY . .

CMD ["nix", "develop", "--command", "bash"]

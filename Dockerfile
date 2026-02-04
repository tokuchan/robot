FROM docker.io/nixos/nix:latest as nix-builder

WORKDIR /workspace

RUN mkdir -p /etc/nix && echo "experimental-features = nix-command flakes" >> /etc/nix/nix.conf

COPY flake.nix flake.lock* ./

RUN if [ ! -f flake.lock ]; then nix flake update; fi

# Pre-populate the Nix store by entering the development environment
RUN nix develop --command bash -c "echo 'Nix environment loaded'"

# Final stage with cached environment
FROM nix-builder

COPY . .

CMD ["nix", "develop", "--command", "bash"]

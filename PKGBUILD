# Maintainer: Zoheb Shujauddin <zoheb2424@gmail.com>
pkgname=game-of-ur
pkgver=0.4.2
pkgrel=1
epoch=
pkgdesc="A 3D adaptation of an ancient, competitive board game."
arch=('x86_64')
url="https://www.github.com/raynmetal/game-of-ur"
license=('MIT')
groups=()
depends=(
    'toymaker'
)
makedepends=('cmake')
checkdepends=()
optdepends=()
provides=()
conflicts=()
replaces=()
backup=()
options=()
install=
changelog=
source=(
    "$pkgname-$pkgver-Source.tar.gz"
)
noextract=()
sha256sums=('55c23fcb41adcddf6fc55783bf8490ba059529d370f247e6e2765da7eadeff3f')
validpgpkeys=()

build() {
    local cmake_options=(
        -B build
        -S $pkgname-$pkgver-Source
        -W no-author
        -D CMAKE_INSTALL_PREFIX=/usr
    )
    cmake "${cmake_options[@]}"
    cmake --build build
}

package() {
    DESTDIR="$pkgdir/" cmake --install build
}

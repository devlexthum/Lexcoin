#!/bin/sh

TOPDIR=${TOPDIR:-$(git rev-parse --show-toplevel)}
SRCDIR=${SRCDIR:-$TOPDIR/src}
MANDIR=${MANDIR:-$TOPDIR/doc/man}

LEXCOIND=${LEXCOIND:-$SRCDIR/lexd}
LEXCOINCLI=${LEXCOINCLI:-$SRCDIR/LEXCOIN-cli}
LEXCOINTX=${LEXCOINTX:-$SRCDIR/LEXCOIN-tx}
LEXCOINQT=${LEXCOINQT:-$SRCDIR/qt/LEXCOIN-qt}

[ ! -x $LEXCOIND ] && echo "$LEXCOIND not found or not executable." && exit 1

# The autodetected version git tag can screw up manpage output a little lex
LEXVER=($($LEXCOINCLI --version | head -n1 | awk -F'[ -]' '{ print $6, $7 }'))

# Create a footer file with copyright content.
# This gets autodetected fine for lexcoind if --version-string is not set,
# but has different outcomes for lexcoin-qt and lexcoin-cli.
echo "[COPYRIGHT]" > footer.h45m
$LEXCOIND --version | sed -n '1!p' >> footer.h45m

for cmd in $LEXCOIND $LEXCOINCLI $LEXCOINTX $LEXCOINQT; do
  cmdname="${cmd##*/}"
  help2man -N --version-string=${BTCVER[0]} --include=footer.h2m -o ${MANDIR}/${cmdname}.1 ${cmd}
  sed -i "s/\\\-${LEXVER[1]}//g" ${MANDIR}/${cmdname}.1
done

rm -f footer.h2m

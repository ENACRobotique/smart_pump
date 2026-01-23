# Smart pompe

- Récupérer les submodules:

    `git submodule update --init --recursive`

## Divers

 ```
 sudo usermod -aG dialout robot 
 sudo apt install make
 ```

### ARM GNU toolchain

 - Télécharger la toolchain depuis https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads.
 - Extraire l'archive dans `/opt` et ajouter au PATH le sous-répertoire `bin`.
 
### PERL pour le générateur board.cfg -> board.h

```
sudo apt install libmodern-perl-perl libxml-libxml-perl libfile-which-perl libsyntax-keyword-match-perl
sudo perl -MCPAN -e 'install String::LCSS'
```




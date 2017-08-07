" ----------------------------- MORIK ----------------------------- {
"
" включить сохранение резервных копий
set backup

" сохранять умные резервные копии ежедневно
function! BackupDir()
    " определим каталог для сохранения резервной копии
    let l:backupdir=$HOME.'/.vim/backup/'.
            \substitute(expand('%:p:h'), '^'.$HOME, '~', '')

    " если каталог не существует, создадим его рекурсивно
    if !isdirectory(l:backupdir)
        call mkdir(l:backupdir, 'p', 0700)
    endif

    " переопределим каталог для резервных копий
    let &backupdir=l:backupdir

    " переопределим расширение файла резервной копии
    let &backupext=strftime('~%Y-%m-%d~')
endfunction

" выполним перед записью буффера на диск
autocmd! bufwritepre * call BackupDir()

set tabstop=4
set expandtab
let c_space_errors=1
set background=dark

" ----------------------------- MORIK ----------------------------- }
"


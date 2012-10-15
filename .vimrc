set expandtab
set tabstop=4
set wrap
set linebreak
set mouse=
syntax on
set showcmd
set ruler
set showmatch
set laststatus=2
set title
"set titlestring=%t%(\ %m%)%(\ %r%)%(\ %h%)%(\ %w%)%(\ (%{expand(\»%:p:~:h\»)})%)\ -\ VIM
set confirm
"set statusline=%<%f%h%m%r%=format=%{&fileformat}\ file=%{&fileencoding}\ enc=%{&encoding}\ %b\ 0x%B\ %l,%c%V\ %P
set visualbell
set hidden
set t_Co=256
set background=dark
set number
set smartindent
set autoindent
set shiftwidth=4
"set list
"set cursorline
set pastetoggle=
set autoread
set keymap=russian-jcukenwin
set iminsert=0
set splitbelow
set splitright
"set foldmethod=manual
"set foldmethod=syntax
set hlsearch
set incsearch
set smartcase
set pastetoggle=<F5>
set iminsert=0
" по умолчанию - латинская раскладка при поиске
set imsearch=0
nmap <F2> :w!<CR>
imap <F2> <Esc> :w!<CR>
vmap <F2> <Esc> :w!<CR>
nmap <F10> :q<CR>
imap <F10> <Esc> :q<CR>
vmap <F10> <Esc> :q<CR>
nmap <F4> :Sex<CR>
imap <F4> <Esc> :Sex<CR>
vmap <F4> <Esc> :Sex<CR>
"nmap <F5> :bp<CR>
"imap <F5> <Esc> :bp<CR>
"vmap <F5> <Esc> :bp<CR>
nmap <F6> :bn<CR>
imap <F6> <Esc> :bn<CR>
vmap <F6> <Esc> :bn<CR>

nmap <S-R> :ConqueTermSplit R -q<CR>
"imap <S-R> <Esc> :ConqueTermSplit R<CR>
"vmap <S-R> <Esc> :ConqueTermSplit R<CR>

inoremap <Nul> <C-p>
function s:ForbidReplace()
    if v:insertmode!=#'i'
        call feedkeys("\<Insert>", "n")
    endif
endfunction
augroup ForbidReplaceMode
    autocmd!
    autocmd InsertEnter  * call s:ForbidReplace()
    autocmd InsertChange * call s:ForbidReplace()
augroup END
colorscheme wombat256
map <C-S-PageUp> :tabp<CR>
map <C-S-PageDown> :tabn<CR>

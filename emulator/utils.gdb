define slist
  if $argc == 0
    printf "usage: slist <head_expr> [max]\n"
  end

  set $n = $arg0
  if $argc >= 2
    set $max = (int)$arg1
  else
    set $max = 64
  end

  set $i = 0
  while $n && $i < $max
    printf "#%d @%p\n", $i, $n
    p *$n
    set $n = $n->next
    set $i = $i + 1
  end
end

document slist
  slist <head_expr> [max] — walk a singly-linked list whose link field is 'next'.
end
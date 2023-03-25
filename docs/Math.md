# The size of the VMM structure
We know that for a maximum order $n$, the size of the VMM structure is 1 page in
addition to the size of 512 VMM stuctures
of order $n - 1$.
With the exception of the maximim pages of order $0$, which has a size of 1 page

This can be represented as the recursive sequence:
$$\begin{cases}S_{n+1} = 4K + 512 \bullet S_n \\\\ S_0 = 4K \end{cases}$$

This recursive sequence can then be written in an absolute way as(idk how to
formally prove this one, but it works):
$$S_n = 4K \sum_{i=0}^{n-1} {512^i} + 4K \bullet 512^n$$

Since $512^i$ is a geometric sequence, then
$${\sum^{n-1}_{i=0}} {512^i} = \frac{1 - 512^n}{1 - 512}$$

And so it follows that:
$$S_n = 4K (\frac{1 - 512^n}{1 - 512} + 512^n)$$

Finally:
$$S_n = 4K \frac{1 - 512^{n+1}}{1 - 512}$$

Which is the formula used by the macro `ORDER_STRUCT_SIZE(n)`:
```c
#define ORDER_STRUCT_SIZE(n) \
    (size_t)(MEM_PS * ((1 - powi(512, (n) + 1)) / ( 1 - 512)))
```

In general, for any sequence $(U_n)_n$ of the recursive formula:
$$ \begin{cases} U_{n+1} = K + r U_n  \\\\ U_0 = C  \end{cases}  $$

Where $K$ and $C$ are constants, it follows then that the absolute formula of
$(U_n)_n$ is:
$$ U_n = C \frac{1 - r^n}{1 - r} + K r^n  $$

Additionally, if $C = K$, we can also write the absolute formula more narrowly
as:
$$ U_n = K \frac{1 - r^{n + 1}}{1 - r}  $$

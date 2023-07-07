#ifndef STDINC_LAMBDA_H
#define STDINC_LAMBDA_H

#define lambda(return_type, body_and_args)                                     \
  ({ return_type __fn__ body_and_args __fn__; })

#endif
interface AssertError extends Error {
  name: 'AssertError';
}

export function assertError(msg: string) {
  const error = new Error(msg) as AssertError;
  error.name = 'AssertError';
  return error;
}

export function unknownAssertError(msg: unknown) {
  const error = new Error(String(msg)) as AssertError;
  error.name = 'AssertError';
  return error;
}

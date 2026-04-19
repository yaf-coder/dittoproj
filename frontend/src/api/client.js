const BASE = import.meta.env.VITE_API_URL ?? ''

function token() {
  return localStorage.getItem('ditto_token')
}

async function request(method, path, body) {
  const headers = { 'Content-Type': 'application/json' }
  const t = token()
  if (t) headers['Authorization'] = `Bearer ${t}`

  const res = await fetch(BASE + path, {
    method,
    headers,
    body: body !== undefined ? JSON.stringify(body) : undefined,
  })

  if (!res.ok) {
    const text = await res.text()
    let message = `HTTP ${res.status}`
    try { message = JSON.parse(text).error ?? message } catch {}
    throw new Error(message)
  }

  return res.json()
}

export const api = {
  // Auth
  register:       (data) => request('POST', '/api/auth/register', data),
  login:          (data) => request('POST', '/api/auth/login', data),
  getMe:          ()     => request('GET',  '/api/auth/me'),
  // Profile
  updateProfile:  (data) => request('PUT',  '/api/profiles/me', data),
  enrichProfile:  (data) => request('PUT',  '/api/profiles/me/enrich', data),
  getProfile:     (id)   => request('GET',  `/api/profiles/${id}`),
  // Questions
  randomQuestion: ()              => request('GET',  '/api/questions/random'),
  answerQuestion: (id, answer)    => request('POST', '/api/questions/answer', { question_id: id, answer }),
  // Discover & matching
  discover:       ()     => request('GET',  '/api/discover'),
  like:           (id)   => request('POST', `/api/likes/${id}`),
  getMatches:     ()     => request('GET',  '/api/matches'),
}

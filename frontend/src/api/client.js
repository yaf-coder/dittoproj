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
    throw new Error(text || `HTTP ${res.status}`)
  }

  return res.json()
}

export const api = {
  // Auth
  login:          (data) => request('POST', '/api/auth/login', data),
  getMe:          ()     => request('GET',  '/api/auth/me'),
  // Profile
  updateProfile:  (data) => request('PUT',  '/api/profiles/me', data),
  getProfile:     (id)   => request('GET',  `/api/profiles/${id}`),
  // Discover & matching
  discover:       ()     => request('GET',  '/api/discover'),
  like:           (id)   => request('POST', `/api/likes/${id}`),
  getMatches:     ()     => request('GET',  '/api/matches'),
}

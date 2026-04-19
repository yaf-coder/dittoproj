import { useState } from 'react'
import { useNavigate, Link } from 'react-router-dom'
import { motion } from 'framer-motion'
import { api } from '../api/client'
import { useAuth } from '../store/AuthContext'
import Button from '../components/Button'
import Input  from '../components/Input'

export default function Register() {
  const { login } = useAuth()
  const navigate  = useNavigate()

  const [form, setForm]   = useState({ name: '', phone_number: '', password: '', confirm: '' })
  const [error, setError] = useState('')
  const [loading, setLoading] = useState(false)

  function handleChange(e) {
    setError('')
    setForm(f => ({ ...f, [e.target.name]: e.target.value }))
  }

  async function handleSubmit(e) {
    e.preventDefault()
    if (!form.name.trim())         { setError('Name is required.'); return }
    if (!form.phone_number.trim()) { setError('Phone number is required.'); return }
    if (form.password.length < 8)  { setError('Password must be at least 8 characters.'); return }
    if (form.password !== form.confirm) { setError('Passwords do not match.'); return }

    setLoading(true)
    try {
      const { token } = await api.register({
        name:         form.name.trim(),
        phone_number: form.phone_number.trim(),
        password:     form.password,
      })
      login(token)
      navigate('/onboarding', { replace: true })
    } catch (e) {
      setError(e.message)
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className="h-dvh flex flex-col px-6 bg-white">
      <div className="pt-14 pb-8">
        <motion.h1
          initial={{ opacity: 0, y: 16 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.5 }}
          className="text-[32px] font-bold tracking-tight2 text-apple-text mb-1"
        >
          Create account
        </motion.h1>
        <p className="text-[17px] text-apple-sub">Join Ditto today.</p>
      </div>

      <form onSubmit={handleSubmit} className="flex-1 flex flex-col gap-4">
        <Input
          name="name"
          label="Full name"
          placeholder="Your name"
          type="text"
          autoComplete="name"
          value={form.name}
          onChange={handleChange}
        />
        <Input
          name="phone_number"
          label="Phone number"
          placeholder="+1 (555) 000-0000"
          type="tel"
          autoComplete="tel"
          value={form.phone_number}
          onChange={handleChange}
        />
        <Input
          name="password"
          label="Password"
          placeholder="At least 8 characters"
          type="password"
          autoComplete="new-password"
          value={form.password}
          onChange={handleChange}
        />
        <Input
          name="confirm"
          label="Confirm password"
          placeholder="Repeat password"
          type="password"
          autoComplete="new-password"
          value={form.confirm}
          onChange={handleChange}
        />

        {error && (
          <motion.p
            initial={{ opacity: 0, y: -6 }}
            animate={{ opacity: 1, y: 0 }}
            className="text-apple-red text-[13px] px-1"
          >
            {error}
          </motion.p>
        )}

        <div className="mt-auto pb-10 flex flex-col gap-3">
          <Button fullWidth type="submit" disabled={loading}>
            {loading ? 'Creating account…' : 'Continue'}
          </Button>
          <p className="text-center text-[15px] text-apple-sub">
            Already have an account?{' '}
            <Link to="/login" className="text-apple-blue font-medium">Log in</Link>
          </p>
        </div>
      </form>
    </div>
  )
}

import { useState } from 'react'
import { motion } from 'framer-motion'
import { useNavigate } from 'react-router-dom'
import { useAuth } from '../store/AuthContext'
import Button from '../components/Button'
import { api } from '../api/client'

const EDUCATION_LABELS = {
  high_school:  'High School',
  some_college: 'Some College',
  bachelors:    "Bachelor's",
  masters:      "Master's",
  phd:          'PhD / Doctorate',
}

const GENDER_LABELS = {
  male:             'Man',
  female:           'Woman',
  non_binary:       'Non-binary',
  other:            'Other',
  prefer_not_to_say: 'Prefer not to say',
}

const POLITICAL_LABELS = [
  { max: 0.125,  label: 'Very Liberal' },
  { max: 0.375,  label: 'Liberal' },
  { max: 0.625,  label: 'Moderate' },
  { max: 0.875,  label: 'Conservative' },
  { max: 1.001,  label: 'Very Conservative' },
]

function politicalLabel(score) {
  for (const { max, label } of POLITICAL_LABELS) {
    if (score < max) return label
  }
  return 'Very Conservative'
}

function Row({ label, value }) {
  if (value === null || value === undefined || value === '') return null
  return (
    <div className="flex items-center justify-between py-3.5 border-b border-apple-gray-2 last:border-0">
      <span className="text-[15px] text-apple-sub">{label}</span>
      <span className="text-[15px] font-medium text-apple-text text-right max-w-[60%]">{value}</span>
    </div>
  )
}

function BeliefBar({ score }) {
  return (
    <div className="py-3.5 border-b border-apple-gray-2 last:border-0">
      <div className="flex items-center justify-between mb-2">
        <span className="text-[15px] text-apple-sub">Political identity</span>
        <span className="text-[15px] font-medium text-apple-text">{politicalLabel(score)}</span>
      </div>
      <div className="flex justify-between text-[11px] text-apple-sub/60 mb-1.5">
        <span>Liberal</span>
        <span>Conservative</span>
      </div>
      <div className="h-1.5 bg-apple-gray-2 rounded-full overflow-hidden">
        <motion.div
          className="h-full rounded-full bg-apple-blue"
          initial={{ width: 0 }}
          animate={{ width: `${score * 100}%` }}
          transition={{ duration: 0.8, ease: [0.25, 0.46, 0.45, 0.94] }}
        />
      </div>
    </div>
  )
}

export default function Profile() {
  const { user, logout, loadUser } = useAuth()
  const navigate = useNavigate()
  const [photoUrl, setPhotoUrl]   = useState('')
  const [photoSaving, setPhotoSaving] = useState(false)
  const [photoFlash, setPhotoFlash]   = useState(false)
  if (!user) return null

  async function handlePhotoSave() {
    if (!photoUrl.trim()) return
    setPhotoSaving(true)
    try {
      await api.updatePhoto(photoUrl.trim())
      await loadUser()
      setPhotoUrl('')
      setPhotoFlash(true)
      setTimeout(() => setPhotoFlash(false), 2000)
    } catch {}
    setPhotoSaving(false)
  }

  const hasEnrichment = user.gender || user.education_level ||
    user.study_location || user.political_belief != null

  return (
    <div className="flex-1 flex flex-col overflow-y-auto">
      <div className="pt-5" />

      {/* Header */}
      <div className="px-6 pt-10 pb-6 flex flex-col items-center text-center">
        <div className="w-24 h-24 rounded-full overflow-hidden bg-apple-gray mb-4 shadow-apple-sm">
          {user.picture ? (
            <img src={user.picture} alt={user.name} className="w-full h-full object-cover" />
          ) : (
            <div className="w-full h-full flex items-center justify-center">
              <span className="text-3xl font-bold text-apple-sub/40">{user.name?.[0]}</span>
            </div>
          )}
        </div>
        <h1 className="text-[24px] font-cursive text-apple-text">{user.name}</h1>
        {user.location && (
          <p className="text-apple-sub text-[15px] mt-0.5 flex items-center gap-1">
            <svg width="12" height="12" viewBox="0 0 24 24" fill="currentColor">
              <path d="M12 2C8.13 2 5 5.13 5 9c0 5.25 7 13 7 13s7-7.75 7-13c0-3.87-3.13-7-7-7zm0 9.5a2.5 2.5 0 0 1 0-5 2.5 2.5 0 0 1 0 5z"/>
            </svg>
            {user.location}
          </p>
        )}
      </div>

      <div className="px-4 pb-10 flex flex-col gap-4">
        {/* Photo URL */}
        <div className="bg-white rounded-3xl shadow-apple-sm px-5 py-4 flex flex-col gap-3">
          <p className="text-[13px] font-semibold text-apple-sub uppercase tracking-wide">Photo</p>
          <div className="flex gap-2">
            <input
              type="url"
              placeholder="Paste a photo URL…"
              value={photoUrl}
              onChange={e => setPhotoUrl(e.target.value)}
              className="flex-1 text-[15px] border border-apple-gray-2 rounded-xl px-3 py-2 outline-none focus:border-apple-blue"
            />
            <button
              onClick={handlePhotoSave}
              disabled={photoSaving || !photoUrl.trim()}
              className="px-4 py-2 rounded-xl bg-apple-blue text-white text-[14px] font-medium disabled:opacity-40"
            >
              {photoFlash ? 'Saved ✓' : 'Save'}
            </button>
          </div>
        </div>

        {/* Basic info */}
        <div className="bg-white rounded-3xl shadow-apple-sm px-5 py-1">
          <Row label="Age"          value={user.age} />
          <Row label="Phone"        value={user.phone_number} />
        </div>

        {/* Enrichment */}
        <div>
          <div className="flex items-center justify-between px-2 mb-2">
            <p className="text-[13px] font-semibold text-apple-sub uppercase tracking-wide">
              About you
            </p>
            <button
              onClick={() => navigate('/questions')}
              className="text-[13px] text-apple-blue font-medium"
            >
              Edit
            </button>
          </div>

          {hasEnrichment ? (
            <div className="bg-white rounded-3xl shadow-apple-sm px-5 py-1">
              <Row label="Gender"        value={GENDER_LABELS[user.gender]} />
              <Row label="Education"     value={EDUCATION_LABELS[user.education_level]} />
              <Row label="Studied at"    value={user.study_location} />
              {user.political_belief != null && <BeliefBar score={user.political_belief} />}
            </div>
          ) : (
            <button
              onClick={() => navigate('/questions')}
              className="w-full bg-white rounded-3xl shadow-apple-sm px-5 py-4 flex items-center justify-between"
            >
              <span className="text-[15px] text-apple-sub">Add details about yourself</span>
              <svg width="16" height="16" viewBox="0 0 24 24" fill="none">
                <path d="M9 18l6-6-6-6" stroke="#C7C7CC" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"/>
              </svg>
            </button>
          )}
        </div>

        <Button variant="secondary" fullWidth onClick={logout}>
          Sign out
        </Button>
      </div>
    </div>
  )
}
